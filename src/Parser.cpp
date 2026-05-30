#include "Parser.h"

#include <format>
#include <sstream>

ParserOutput Parser::Parse(const std::vector<Token>& tokens) {
    Context context;
    context.tokens = tokens;

    while(peek(context)) {
        if(context.error)
            return context.error.value();

        parseIteration(context);
    }

    return std::move(context.rootNode);
}

void Parser::parseIteration(Context& context) {
    if(is<NewlineToken>(context)) {
        consume(context);
        return;
    }

    if(is<KeywordToken>(context)) {
        if(as<KeywordToken>(context).cmd == "if") {
            consume(context);
            expect<ParenthesesToken>(context);

            parseIf(context);
            return;
        }

        if(as<KeywordToken>(context).cmd == "while") {
            consume(context);
            expect<ParenthesesToken>(context);

            parseWhile(context);
            return;
        }

        if(as<KeywordToken>(context).cmd == "for") {
            consume(context);
            expect<ParenthesesToken>(context);

            parseFor(context);

            return;
        }

        size_t i=1;
        while(is<KeywordToken,StringToken,VariableToken,NumericToken>(context,i++));
        if(is<AlgebraicOperatorToken,LogicalOperatorToken>(context,--i)) {
            parseAlgebraicExpression(context);
            return;
        }

        if(parseCmd(context))
            return;
    }

    if(is<SignToken>(context)) {
        const Token token = *peek(context);
        parseRedirect(context, context.lastNode, true, token.x, token.y);
        return;
    }

    if(is<VariableToken>(context)) {
        if(is<NumericToken,AlgebraicOperatorToken>(context,1))
            parseAlgebraicExpression(context);
        else
            parseVar(context);

        return;
    }

    if(is<FuncToken>(context)) {
        parseFunc(context);
        return;
    }

    if(is<NumericToken,AlgebraicOperatorToken,ParenthesesToken>(context)) {
        parseAlgebraicExpression(context);
        return;
    }

    if(is<StringToken>(context)) {
        if(is<AlgebraicOperatorToken,LogicalOperatorToken>(context,1)) {
            parseAlgebraicExpression(context);
            return;
        }

        if(is<SignToken>(context,1)) {
            const auto cn = StringNode(as<StringToken>(context).value);
            const Token ct = *peek(context);
            consume(context);

            parseRedirect(context, std::make_shared<Node>(cn), true, ct.x, ct.y);
            return;
        }

        context.rootNode.nodes.emplace_back(StringNode(as<StringToken>(context).value),  peek(context)->x, peek(context)->y);
        consume(context);
        return;
    }

    consume(context);
}

std::optional<Token> Parser::peek(const Context& context, const size_t offset) const {
    return context.idx + offset < context.tokens.size()
        ? std::optional{context.tokens[context.idx+offset]}
           : std::nullopt;
}

std::optional<Token> Parser::consume(Context& context) {
    std::optional<Token> token = peek(context);

    if(token)
        ++context.idx;

    if(!context.rootNode.nodes.empty())
        context.lastNode = std::make_shared<Node>(context.rootNode.nodes.back());

    return token;
}

template<typename ... Ts>
bool Parser::is(const Context& context, const size_t offset) const {
    const std::optional<Token> token = peek(context, offset);
    return token.has_value() && (std::holds_alternative<Ts>(token.value().value) || ...);
}

template<typename T>
T Parser::as(const Context& context) const {
    return std::get<T>(peek(context).value().value);
}

template<typename T>
bool Parser::expect(Context& context) const {
    if(const std::optional<Token> token = peek(context); !token.has_value()) {
        context.error = ParserError("Unexpected end of input", context.tokens.back().x, context.tokens.back().y);
        return false;
    }

    if(!is<T>(context)) {
        const Token token = *peek(context);
        context.error = ParserError("Unexpected token type", token.x, token.y);
        return false;
    }

    return true;
}

void Parser::parseFunc(Context& context) {
    const std::string funcName = as<FuncToken>(context).name;
    consume(context);

    expect<ParenthesesToken>(context);
    consume(context);
    std::vector<Node> vars;
    bool isVar = true;

    while(is<StringToken,VariableToken>(context)) {
        if(is<VariableToken>(context)) {
            std::visit([&](const auto& t) { vars.emplace_back(t); }, parseVar(context, false));
            continue;
        }

        isVar = false;

        if(is<StringToken>(context)) {
            vars.emplace_back(StringNode(as<StringToken>(context).value));
            consume(context);
            continue;
        }
    }

    consume(context);

    if(is<FuncParenthesesToken>(context) && isVar) {
        FuncDeclarationNode fdn;
        fdn.name = funcName;

        fdn.vars.resize(vars.size());
        std::transform(vars.begin(), vars.end(), fdn.vars.begin(), [](const Node& node) {
            return std::visit([&]<typename T>(const T& t) -> std::variant<VarNode,ArrNode> {
                if constexpr(std::is_same_v<std::decay_t<T>,VarNode> ||
                             std::is_same_v<std::decay_t<T>,ArrNode>)
                    return t;
                else
                    throw std::logic_error("Unreachable");
            }, node.value);
        });

        fdn.body = std::get<RootNode>(Parse(getBody(context)));

        context.rootNode.nodes.emplace_back(fdn);
        return;
    }

    FuncCallNode fcn;
    fcn.name = funcName;
    fcn.params = std::move(vars);

    context.rootNode.nodes.emplace_back(fcn);
}

std::optional<Node> Parser::parseCmd(Context& context, const bool push, const bool operandMode) {
    if(!is<KeywordToken>(context))
        return std::nullopt;

    const Token token = *peek(context);

    auto cn = CmdNode(as<KeywordToken>(context).cmd);
    consume(context);

    while(is<KeywordToken,StringToken,VariableToken,NumericToken,AlgebraicOperatorToken,ParenthesesToken>(context)) {
        const Token targ = *peek(context);

        if(operandMode && is<AlgebraicOperatorToken,LogicalOperatorToken>(context))
            break;

        if(is<KeywordToken>(context)) {
            if(!operandMode && is<AlgebraicOperatorToken,LogicalOperatorToken>(context,1))
                cn.args.emplace_back(parseAlgebraicExpression(context,false));
            else {
                cn.args.emplace_back(ArgNode(as<KeywordToken>(context).cmd), targ.x, targ.y);
                consume(context);
            }
        }
        else if(is<StringToken>(context)) {
            const std::string& str = as<StringToken>(context).value;
            std::istringstream iss (str.data());
            ArgNode tan;

            while(iss >> tan.arg)
                cn.args.emplace_back(tan, targ.x, targ.y);

            consume(context);
        }
        else if(is<VariableToken>(context)) {
            if(!operandMode && is<AlgebraicOperatorToken,LogicalOperatorToken>(context,1))
                cn.args.emplace_back(parseAlgebraicExpression(context, false));
            else {
                Node node;
                std::visit([&node](const auto& t){ node = Node(t); }, parseVar(context, false));

                if(!operandMode && is<AlgebraicOperatorToken,LogicalOperatorToken>(context))
                    cn.args.emplace_back(parseAlgebraicExpression(context, false, &node));
                else
                    cn.args.emplace_back(std::move(node));
            }
        }
        else
            cn.args.emplace_back(parseAlgebraicExpression(context,false));
    }

    if(is<SignToken>(context)) {
        parseRedirect(context, std::make_shared<Node>(cn), push, token.x, token.y);
        return Node(std::move(cn), token.x, token.y);
    }

    if(push)
        context.rootNode.nodes.emplace_back(std::move(cn), token.x, token.y);

    return Node(std::move(cn), token.x, token.y);
}

std::optional<RedirectNode> Parser::parseRedirect(Context& context, std::shared_ptr<Node> node, const bool push, const size_t x, const size_t y) {
    if(!is<SignToken>(context))
        return std::nullopt;

    const Sign sign = as<SignToken>(context).sign;
    consume(context);

    if(std::holds_alternative<VarNode>(node->value)
    || std::holds_alternative<ArrNode>(node->value))
        context.rootNode.nodes.pop_back();

    if(auto scn = parseCmd(context, false)) {
        auto rn = RedirectNode {
            node,
            std::make_shared<Node>(std::move(*scn)),
            sign
        };

        if(push)
            context.rootNode.nodes.emplace_back(std::move(rn), x, y);

        return rn;
    }

    if(is<VariableToken>(context)) {
        RedirectNode rn;
        bool doConsume = true;

        if(is<NumericToken,AlgebraicOperatorToken>(context,1)) {
            rn = RedirectNode {
                node,
                std::make_shared<Node>(parseAlgebraicExpression(context,false)),
                sign
            };

            doConsume = false;
        }
        else {
            std::shared_ptr<Node> tn;
            std::visit([&tn](const auto& t) {
                tn = std::make_shared<Node>(t);
            }, parseVar(context, false));

            if(is<AlgebraicOperatorToken,LogicalOperatorToken>(context))
                tn = std::make_shared<Node>(parseAlgebraicExpression(context, false, tn.get()));

            rn = RedirectNode {
                node,
                tn,
                sign
            };
            doConsume = false;
        }

        if(push)
            context.rootNode.nodes.emplace_back(std::move(rn), x, y);

        if(doConsume)
            consume(context);

        return rn;
    }

    if(is<NumericToken,AlgebraicOperatorToken,ParenthesesToken>(context)) {
        auto rn = RedirectNode {
            node,
            std::make_shared<Node>(parseAlgebraicExpression(context,false)),
            sign
        };

        if(push)
            context.rootNode.nodes.emplace_back(std::move(rn), x, y);

        return rn;
    }

    if(is<StringToken>(context)) {
        auto rn = RedirectNode {
            node,
            std::make_shared<Node>(StringNode(as<StringToken>(context).value)),
            sign
        };

        if(push)
            context.rootNode.nodes.emplace_back(std::move(rn), x, y);

        consume(context);
        return rn;
    }
}

std::variant<VarNode,ArrNode> Parser::parseVar(Context& context, const bool push) {
    const Token token = *peek(context);
    if(is<MiscParenthesesToken>(context,1)
    && std::get<MiscParenthesesToken>(peek(context,1)->value).value == MiscParentheses::BodyOpen) {
        const std::string arrNode = as<VariableToken>(context).name;

        consume(context);
        consume(context);
        const AlgebraicNode an = parseAlgebraicExpression(context, false);

        auto tan = ArrNode(arrNode, an);

        if(push)
            context.rootNode.nodes.emplace_back(tan);

        consume(context);
        return tan;
    }

    auto tvn = VarNode(as<VariableToken>(context).name);

    if(push)
        context.rootNode.nodes.emplace_back(tvn, token.x, token.y);

    consume(context);
    return tvn;
}

AlgebraicNode Parser::parseAlgebraicExpression(Context& context, const bool push, const Node* node) {
    AlgebraicNode an;

    if(node != nullptr)
        an.tns.emplace_back(*node);

    const Token first = *peek(context);

    while(is<KeywordToken,NumericToken,AlgebraicOperatorToken,VariableToken,ParenthesesToken,LogicalOperatorToken,StringToken>(context)) {
        if(is<KeywordToken>(context)) {
            an.tns.emplace_back(*parseCmd(context, false, true));
            continue;
        }

        if(is<StringToken>(context)) {
            an.tns.emplace_back(Token{as<StringToken>(context), peek(context)->x, peek(context)->y});
            consume(context);
            continue;
        }

        if(is<VariableToken>(context))
            std::visit([&] (const auto& t) { an.tns.emplace_back(Node(t)); }, parseVar(context, false));
        else {
            an.tns.emplace_back(*peek(context));
            consume(context);
        }
    }

    if(push && is<SignToken>(context)) {
        parseRedirect(context, std::make_shared<Node>(an), push, first.x, first.y);
        return an;
    }

    if(push)
        context.rootNode.nodes.emplace_back(an);

    return an;
}

IfNode Parser::parseIf(Context& context, const bool push) {
    const AlgebraicNode condition = parseAlgebraicExpression(context, false);

    const std::vector<Token> ifBody = getBody(context);
    RootNode elseBody;

    while(is<NewlineToken>(context))
        consume(context);

    if(is<KeywordToken>(context) && as<KeywordToken>(context).cmd == "else") {
        if(is<KeywordToken>(context) && as<KeywordToken>(context).cmd == "if") {
            consume(context);
            elseBody.nodes.emplace_back(parseIf(context,false));
        }
        else {
            consume(context);
            elseBody = std::get<RootNode>(Parse(getBody(context)));
        }
    }

    const auto in = IfNode(
        condition,
        std::get<RootNode>(Parse(ifBody)),
        elseBody
    );

    if(is<SignToken>(context)) {
        const Token token = *peek(context);
        parseRedirect(context, std::make_shared<Node>(in), push, token.x, token.y);
        return in;
    }

    if(push)
        context.rootNode.nodes.emplace_back(in);

    return in;
}

void Parser::parseWhile(Context& context) {
    const AlgebraicNode condition = parseAlgebraicExpression(context, false);

    const std::vector<Token> body = getBody(context);

    const auto wn = WhileNode(condition, std::get<RootNode>(Parse(body)));

    if(is<SignToken>(context)) {
        const Token token = *peek(context);
        parseRedirect(context, std::make_shared<Node>(wn), true, token.x, token.y);
        return;
    }

    context.rootNode.nodes.emplace_back(wn);
}

void Parser::parseFor(Context& context) {
    const Token token = *peek(context);
    consume(context);

    parseIteration(context);

    if(context.lastNode != nullptr && !(std::holds_alternative<VarNode>(context.lastNode->value)
    || std::holds_alternative<ArrNode>(context.lastNode->value)))
        context.rootNode.nodes.pop_back();

    const std::optional<RedirectNode> declaration = parseRedirect(context, context.lastNode, false, token.x, token.y);

    if(declaration)
        consume(context);

    const AlgebraicNode condition = parseAlgebraicExpression(context, false);

    consume(context);

    const bool isEmptyIteration = is<ParenthesesToken>(context);

    if(isEmptyIteration)
        consume(context);
    else
        parseIteration(context);

    if(context.lastNode != nullptr && !(std::holds_alternative<VarNode>(context.lastNode->value)
    || std::holds_alternative<ArrNode>(context.lastNode->value)))
        context.rootNode.nodes.pop_back();

    if(!isEmptyIteration)
        context.tokens.insert(
            context.tokens.begin() + context.idx + 1,
            Token{ ParenthesesToken(Parentheses::FuncOpen), token.x, token.y }
        );

    const std::optional<RedirectNode> iteration = parseRedirect(context, context.lastNode, false, token.x, token.y);

    const std::vector<Token> body = getBody(context);

    context.rootNode.nodes.emplace_back(ForNode(declaration, iteration, condition, std::get<RootNode>(Parse(body))));
}

std::vector<Token> Parser::getBody(Context& context) {
    bool singleLineMode = false;

    if(is<FuncParenthesesToken>(context))
        consume(context);
    else {
        singleLineMode = true;
        consume(context);
    }

    int depth = 1;
    std::vector<Token> body;

    while(!singleLineMode ? depth != 0 : !is<NewlineToken>(context)) {
        auto opt = peek(context);
        if(!opt) break;
        const Token& t = *opt;

        if(is<FuncParenthesesToken>(context)) {
            if(const FuncParentheses spt = as<FuncParenthesesToken>(context).value;
                spt == FuncParentheses::BodyOpen) {
                ++depth;
                body.push_back(t);
            }
            else if(spt == FuncParentheses::BodyClose) {
                if(--depth != 0) body.push_back(t);
            } else body.push_back(t);
        } else
            body.push_back(t);

        consume(context);
    }

    return body;
}
