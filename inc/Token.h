#ifndef SHELLANG_TOKEN_H
#define SHELLANG_TOKEN_H

#include <string>
#include <variant>
#include <unordered_map>
#include <string_view>

struct KeywordToken {
    static constexpr std::string_view KEYWORD_CHARS = "_-.";

    std::string cmd;
    explicit KeywordToken(std::string_view);
};

struct VariableToken {
    static constexpr char SIGN = '$';

    std::string name;
    explicit VariableToken(std::string_view);
};

struct NumericToken {
    static constexpr char DECIMAL_CHAR = '.';
    static constexpr std::string_view NUMBER_EXCEPTIONS = "_";

    double value;
    explicit NumericToken(double);
};

struct StringToken {
    static constexpr char QUOTE_CHAR = '"';
    static constexpr char ESCAPE_CHAR = '\\';

    std::string value;
    explicit StringToken(std::string_view);
};

enum class Parentheses { FuncOpen, FuncClose };

struct ParenthesesToken {
    inline static const std::unordered_map<char, Parentheses> PARENTHESES = {
        { '(', Parentheses::FuncOpen },
        { ')', Parentheses::FuncClose }
    };

    Parentheses value;
    explicit ParenthesesToken(Parentheses);
};

enum class AlgebraicOperator { Add, Sub, Mul, Div, Mod, Pow, None };

struct AlgebraicOperatorToken {
    inline static const std::unordered_map<char, AlgebraicOperator> OPERATORS = {
        { '+', AlgebraicOperator::Add },
        { '-', AlgebraicOperator::Sub },
        { '*', AlgebraicOperator::Mul },
        { '/', AlgebraicOperator::Div },
        { '%', AlgebraicOperator::Mod },
        { '^', AlgebraicOperator::Pow }
    };

    AlgebraicOperator op;
    explicit AlgebraicOperatorToken(AlgebraicOperator);
};

enum class LogicalOperator { And, Or, Equals, NotEquals, Not };

struct LogicalOperatorToken {
    static constexpr std::string_view ALL_LOGICAL_OPS = "&|=!";

    static inline const std::unordered_map<std::string, LogicalOperator> LOGICAL_OPS = {
        { "&&", LogicalOperator::And },
        { "||", LogicalOperator::Or },
        { "==", LogicalOperator::Equals },
        { "!=", LogicalOperator::NotEquals },
        { "!", LogicalOperator::Not}
    };

    LogicalOperator op;
    explicit LogicalOperatorToken(LogicalOperator);
};

enum class Sign { RedirectLeft, RedirectRight, AppendLeft, AppendRight };

struct SignToken {
    static constexpr std::string_view ALL_SIGNS = "<>";

    static inline const std::unordered_map<std::string, Sign> SIGNS = {
        { ">", Sign::RedirectRight },
        { "<", Sign::RedirectLeft },
        { ">>", Sign::AppendRight },
        { "<<", Sign::AppendLeft },
    };

    Sign sign;
    explicit SignToken(Sign);
};

struct NewlineToken {
    static constexpr std::string_view NEWLINE_CHARS = ";\n";

    NewlineToken();
};

using TokenValue = std::variant<
    KeywordToken,
    VariableToken,
    NumericToken,
    StringToken,
    ParenthesesToken,
    AlgebraicOperatorToken,
    LogicalOperatorToken,
    SignToken,
    NewlineToken
>;

struct Token {
    TokenValue value;
    size_t x, y;
};

#endif
