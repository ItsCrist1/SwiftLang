#include "Parser.h"

#include <format>
#include <sstream>

ParserOutput Parser::Parse(const std::vector<Token>& tokens) {
    Context context;
    context.tokens = tokens;

    while(peek(context)) {
        if(context.error)
            return context.error.value();

        if(is<NewlineToken>(context)) {
            consume(context);
            continue;
        }

        if(is<KeywordToken>(context)) {
            if(as<KeywordToken>(context).cmd == "if") {
                consume(context);
                expect<ParenthesesToken>(context);

                parseIf(context);
                continue;
            }

            if(as<KeywordToken>(context).cmd == "while") {
                consume(context);
                expect<ParenthesesToken>(context);

                parseWhile(context);
                continue;
            }

            size_t i=1;
            while(is<KeywordToken,StringToken,VariableToken,NumericToken>(context,i++));
            if(is<AlgebraicOperatorToken,LogicalOperatorToken>(context,--i)) {
                parseAlgebraicExpression(context);
                continue;
            }

            if(parseCmd(context))
                continue;
        }

        if(is<SignToken>(context)) {
            const Token token = *peek(context);
            parseRedirect(context, context.lastNode, true, token.x, token.y);
            continue;
        }

        if(is<VariableToken>(context)) {
            if(is<NumericToken,AlgebraicOperatorToken>(context,1))
                parseAlgebraicExpression(context);
            else
                parseVar(context);

            continue;
        }

        if(is<NumericToken,AlgebraicOperatorToken,ParenthesesToken>(context)) {
            parseAlgebraicExpression(context);
            continue;
        }

        if(is<StringToken>(context)) {
            if(is<AlgebraicOperatorToken,LogicalOperatorToken>(context,1)) {
                parseAlgebraicExpression(context);
                continue;
            }

            if(is<SignToken>(context,1)) {
                const auto cn = StringNode(as<StringToken>(context).value);
                const Token ct = *peek(context);
                consume(context);

                parseRedirect(context, std::make_shared<Node>(cn), true, ct.x, ct.y);
                continue;
            }

            context.rootNode.nodes.emplace_back(StringNode(as<StringToken>(context).value),  peek(context)->x, peek(context)->y);
            consume(context);
            continue;
        }

        consume(context);
    }

    return std::move(context.rootNode);
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

void Parser::parseRedirect(Context& context, const std::shared_ptr<Node>& node, const bool push, const size_t x, const size_t y) {
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

        if(push) {
            context.rootNode.nodes.emplace_back(std::move(rn), x, y);
            return;
        }
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

            rn = RedirectNode {
                node,
                tn,
                sign
            };
        }

        context.rootNode.nodes.emplace_back(std::move(rn), x, y);

        if(doConsume)
            consume(context);

        return;
    }

    if(is<NumericToken,AlgebraicOperatorToken,ParenthesesToken>(context)) {
        auto rn = RedirectNode {
            node,
            std::make_shared<Node>(parseAlgebraicExpression(context,false)),
            sign
        };

        context.rootNode.nodes.emplace_back(std::move(rn), x, y);
        return;
    }

    if(is<StringToken>(context)) {
        auto rn = RedirectNode {
            node,
            std::make_shared<Node>(StringNode(as<StringToken>(context).value)),
            sign
        };

        context.rootNode.nodes.emplace_back(std::move(rn), x, y);
        consume(context);
        return;
    }
}

std::variant<VarNode,ArrNode> Parser::parseVar(Context& context, const bool push) {
    const Token token = *peek(context);
    if(is<MiscParenthesesToken>(context,1)) {
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

    expect<FuncParenthesesToken>(context);
    consume(context);

    const std::vector<Token> ifBody = getBody(context);
    RootNode elseBody;

    while(is<NewlineToken>(context))
        consume(context);

    if(is<KeywordToken>(context) && as<KeywordToken>(context).cmd == "else") {
        consume(context);
        if(is<KeywordToken>(context) && as<KeywordToken>(context).cmd == "if") {
            consume(context);
            elseBody.nodes.emplace_back(parseIf(context,false));
        }
        else
            elseBody = std::get<RootNode>(Parse(getBody(context)));
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

    expect<FuncParenthesesToken>(context);
    consume(context);

    const std::vector<Token> body = getBody(context);

    const auto wn = WhileNode(condition, std::get<RootNode>(Parse(body)));

    if(is<SignToken>(context)) {
        const Token token = *peek(context);
        parseRedirect(context, std::make_shared<Node>(wn), true, token.x, token.y);
        return;
    }

    context.rootNode.nodes.emplace_back(wn);
}

std::vector<Token> Parser::getBody(Context& context) {
    int depth = 1;
    std::vector<Token> body;

    while(depth != 0) {
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
