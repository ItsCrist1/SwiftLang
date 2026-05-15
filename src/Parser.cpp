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

        if(is<SignToken>(context)) {
            const Token token = *peek(context);
            parseRedirect(context, context.lastNode, true, token.x, token.y);
            continue;
        }

        if(is<VariableToken>(context)) {
            const Token token = *peek(context);
            context.rootNode.nodes.emplace_back(VarNode(as<VariableToken>(context).name), token.x, token.y);
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
    if(!is<KeywordToken>(context))
        return std::nullopt;

    const Token token = *peek(context);

    auto cn = CmdNode(as<KeywordToken>(context).cmd);
    consume(context);

    while(is<KeywordToken,VariableToken>(context)) {
        const Token targ = *peek(context);

        if(is<KeywordToken>(context))
            cn.args.emplace_back(ArgNode(as<KeywordToken>(context).cmd), targ.x, targ.y);
        else
            cn.args.emplace_back(VarNode(as<VariableToken>(context).name), targ.x, targ.y);

        consume(context);
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
        auto rn = RedirectNode {
            node,
            std::make_shared<Node>(VarNode(as<VariableToken>(context).name)),
            sign
        };

        context.rootNode.nodes.emplace_back(std::move(rn), x, y);
        consume(context);
        return;
    }
}
