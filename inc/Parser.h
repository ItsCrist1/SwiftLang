#ifndef SHELLANG_PARSER_H
#define SHELLANG_PARSER_H

#include <optional>

#include "Error.h"
#include "Node.h"
#include "Token.h"

using ParserOutput = std::variant<RootNode,ParserError>;

struct Parser {
    ParserOutput Parse(const std::vector<Token>&);

private:
    struct Context {
        RootNode rootNode;
        std::vector<Token> tokens;
        std::optional<ParserError> error;
        size_t idx = 0u;
        std::shared_ptr<Node> lastNode;
    };

    [[nodiscard]] std::optional<Token> peek(const Context&, size_t offset=0u) const;
    std::optional<Token> consume(Context&);

    template<typename ... Ts>
    [[nodiscard]] bool is(const Context&, size_t offset=0u) const;

    template<typename T>
    T as(const Context&) const;

    template<typename T>
    bool expect(Context&) const;

    void parseIteration(Context&);

    std::optional<Node> parseCmd(Context& context, bool push=true, bool operandMode=false);
    std::optional<RedirectNode> parseRedirect(Context&, std::shared_ptr<Node>, bool, size_t, size_t);
    std::variant<VarNode,ArrNode> parseVar(Context&, bool push=true);
    AlgebraicNode parseAlgebraicExpression(Context&, bool push=true, const Node* = nullptr);
    IfNode parseIf(Context&, bool push=true);
    void parseWhile(Context&);
    void parseFor(Context&);

    std::vector<Token> getBody(Context&);
};

#endif
