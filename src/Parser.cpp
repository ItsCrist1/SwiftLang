#include "Parser.h"

#include <format>

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
            if(parseCmd(context))
                continue;
        }
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

    return token;
}

template<typename ... Ts>
bool Parser::is(const Context& context) const {
    const std::optional<Token> token = peek(context);
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
    if(!expect<KeywordToken>(context))
        return std::nullopt;

    const Token token = *peek(context);

    CmdNode cn{as<KeywordToken>(context).cmd};
    consume(context);

    while(is<KeywordToken>(context)) {
        cn.args.emplace_back(ArgNode(as<KeywordToken>(context).cmd), token.x, token.y);
        consume(context);
    }

    if(is<SignToken>(context)) {
        const Sign sign = as<SignToken>(context).sign;
        consume(context);

        if(auto scn = parseCmd(context, false)) {
            auto rn = RedirectNode {
                std::make_shared<Node>(std::move(cn)),
                std::make_shared<Node>(std::move(*scn)),
                sign
            };

            if(push) {
                context.rootNode.nodes.emplace_back(std::move(rn), token.x, token.y);
                return std::nullopt;
            }

            return Node(std::move(rn), token.x, token.y);
        }
    }

    if(push)
        context.rootNode.nodes.emplace_back(std::move(cn), token.x, token.y);

    return Node(std::move(cn), token.x, token.y);
}