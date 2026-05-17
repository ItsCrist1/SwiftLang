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
                expect<SParenthesesToken>(context);

                processIf(context);
                continue;
            }

            if(as<KeywordToken>(context).cmd == "while") {
                consume(context);
                expect<SParenthesesToken>(context);

                processWhile(context);
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

    if (!is<T>(context)) {
        const Token token = *peek(context);
        context.error = ParserError("Unexpected token type", token.x, token.y);
        return false;
    }

    return true;
}

std::optional<Node> Parser::parseCmd(Context& context, const bool push) {
    if(!is<KeywordToken>(context))
        return std::nullopt;

    const Token token = *peek(context);

    auto cn = CmdNode(as<KeywordToken>(context).cmd);
    consume(context);

    while(is<KeywordToken,StringToken,VariableToken,NumericToken,AlgebraicOperatorToken,ParenthesesToken>(context)) {
        const Token targ = *peek(context);

        if(is<KeywordToken>(context)) {
            cn.args.emplace_back(ArgNode(as<KeywordToken>(context).cmd), targ.x, targ.y);
            consume(context);
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
            cn.args.emplace_back(VarNode(as<VariableToken>(context).name), targ.x, targ.y);
            consume(context);
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

    if(std::holds_alternative<VarNode>(node->value))
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
        else
            rn = RedirectNode {
                node,
                std::make_shared<Node>(VarNode(as<VariableToken>(context).name)),
                sign
            };

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
}

void Parser::parseVar(Context& context) {
    const Token token = *peek(context);
    context.rootNode.nodes.emplace_back(VarNode(as<VariableToken>(context).name), token.x, token.y);
    consume(context);
}

AlgebraicNode Parser::parseAlgebraicExpression(Context& context, const bool push) {
    AlgebraicNode an;
    const Token first = *peek(context);

    while(is<KeywordToken,NumericToken,AlgebraicOperatorToken,VariableToken,ParenthesesToken,LogicalOperatorToken>(context)) {
        if(is<KeywordToken>(context)) {
            an.tns.emplace_back(*parseCmd(context, false));
            continue;
        }

        an.tns.emplace_back(*peek(context));
        consume(context);
    }

    if(push && is<SignToken>(context)) {
        parseRedirect(context, std::make_shared<Node>(an), push, first.x, first.y);
        return an;
    }

    if(push)
        context.rootNode.nodes.emplace_back(an);

    return an;
}

IfNode Parser::processIf(Context& context, const bool push) {
    consume(context);
    const AlgebraicNode condition = parseAlgebraicExpression(context, false);
    consume(context);

    const std::vector<Token> ifBody = getBody(context);
    RootNode elseBody;

    while(is<NewlineToken>(context))
        consume(context);

    if(is<KeywordToken>(context) && as<KeywordToken>(context).cmd == "else") {
        consume(context);
        if(is<KeywordToken>(context) && as<KeywordToken>(context).cmd == "if") {
            consume(context);
            elseBody.nodes.emplace_back(processIf(context,false));
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

void Parser::processWhile(Context& context) {
    consume(context);
    const AlgebraicNode condition = parseAlgebraicExpression(context, false);
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
        if (!opt) break;
        const Token t = *opt;

        if(is<SParenthesesToken>(context)) {
            if(const SParentheses spt = as<SParenthesesToken>(context).value;
                spt == SParentheses::FuncClose) {
                ++depth;
                body.push_back(t);
            }
            else if(spt == SParentheses::BodyClose) {
                if(--depth != 0) body.push_back(t);
            } else body.push_back(t);
        } else
            body.push_back(t);

        consume(context);
    }

    return body;
}
