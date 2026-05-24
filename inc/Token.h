#ifndef SHELLANG_TOKEN_H
#define SHELLANG_TOKEN_H

#include <string>
#include <variant>
#include <unordered_map>
#include <string_view>

struct KeywordToken {
    static constexpr std::string_view KEYWORD_CHARS = "_-./\\~";

    std::string cmd;
    explicit KeywordToken(std::string_view);

    bool operator==(const KeywordToken&) const = default;
};

struct VariableToken {
    static constexpr char SIGN = '$';

    std::string name;
    explicit VariableToken(std::string_view);

    bool operator==(const VariableToken&) const = default;
};

struct FuncToken {
    static constexpr char SIGN = '@';

    std::string name;
    explicit FuncToken(std::string_view);

    bool operator==(const FuncToken&) const = default;
};

struct NumericToken {
    static constexpr char DECIMAL_CHAR = '.';
    static constexpr std::string_view NUMBER_EXCEPTIONS = "_";

    double value;
    explicit NumericToken(double);

    bool operator==(const NumericToken&) const = default;
};

struct StringToken {
    static constexpr char QUOTE_CHAR = '"';
    static constexpr char ESCAPE_CHAR = '\\';

    std::string value;
    explicit StringToken(std::string_view);

    bool operator==(const StringToken&) const = default;
};

enum class Parentheses { FuncOpen, FuncClose };

struct ParenthesesToken {
    inline static const std::unordered_map<char, Parentheses> PARENTHESES = {
        { '(', Parentheses::FuncOpen },
        { ')', Parentheses::FuncClose }
    };

    Parentheses value;
    explicit ParenthesesToken(Parentheses);

    bool operator==(const ParenthesesToken&) const = default;
};

enum class MiscParentheses { BodyOpen, BodyClose };

struct MiscParenthesesToken {
    inline static const std::unordered_map<char, MiscParentheses> PARENTHESES = {
        { '[', MiscParentheses::BodyOpen },
        { ']', MiscParentheses::BodyClose }
    };

    MiscParentheses value;
    explicit MiscParenthesesToken(MiscParentheses);

    bool operator==(const MiscParenthesesToken&) const = default;
};

enum class FuncParentheses { BodyOpen, BodyClose };

struct FuncParenthesesToken {
    inline static const std::unordered_map<char, FuncParentheses> PARENTHESES = {
        { '{', FuncParentheses::BodyOpen },
        { '}', FuncParentheses::BodyClose }
    };

    FuncParentheses value;
    explicit FuncParenthesesToken(FuncParentheses);

    bool operator==(const FuncParenthesesToken&) const = default;
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

    bool operator==(const AlgebraicOperatorToken&) const = default;
};

enum class LogicalOperator { And, Or, Equals, NotEquals, Not, Lesser, LesserEquals, Greater, GreaterEquals };

struct LogicalOperatorToken {
    static constexpr std::string_view ALL_LOGICAL_OPS = "&|=!<>_";

    static inline const std::unordered_map<std::string, LogicalOperator> LOGICAL_OPS = {
        { "&&", LogicalOperator::And },
        { "||", LogicalOperator::Or },
        { "==", LogicalOperator::Equals },
        { "!=", LogicalOperator::NotEquals },
        { "!", LogicalOperator::Not },
        { "<", LogicalOperator::Lesser },
        { "<=", LogicalOperator::LesserEquals },
        { ">", LogicalOperator::Greater },
        { ">=", LogicalOperator::GreaterEquals }
    };

    LogicalOperator op;
    explicit LogicalOperatorToken(LogicalOperator);

    bool operator==(const LogicalOperatorToken&) const = default;
};

enum class Sign { RedirectLeft, RedirectRight, AppendLeft, AppendRight };

struct SignToken {
    static constexpr std::string_view ALL_SIGNS = "-<>";

    static inline const std::unordered_map<std::string, Sign> SIGNS = {
        { "->", Sign::RedirectRight },
        { "<-", Sign::RedirectLeft },
        { ">>", Sign::AppendRight },
        { "<<", Sign::AppendLeft },
    };

    Sign sign;
    explicit SignToken(Sign);

    bool operator==(const SignToken&) const = default;
};

struct NewlineToken {
    static constexpr std::string_view NEWLINE_CHARS = ";\n";

    NewlineToken();

    bool operator==(const NewlineToken&) const = default;
};

using TokenValue = std::variant<
    KeywordToken,
    VariableToken,
    FuncToken,
    NumericToken,
    StringToken,
    ParenthesesToken,
    MiscParenthesesToken,
    FuncParenthesesToken,
    AlgebraicOperatorToken,
    LogicalOperatorToken,
    SignToken,
    NewlineToken
>;

struct Token {
    TokenValue value;
    size_t x, y;

    bool operator==(const Token& o) const { return value == o.value; }
};

#endif
