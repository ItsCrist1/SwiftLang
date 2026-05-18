#ifndef SHELLANG_LEXER_H
#define SHELLANG_LEXER_H

#include "Error.h"
#include "Token.h"

#include <string>
#include <vector>
#include <memory>
#include <optional>

using LexerOutput = std::variant<std::vector<Token>, LexerError>;

class Lexer {
public:
    Lexer();

    LexerOutput Lex(std::string_view);
private:
    struct Context {
        enum class State { Search, Keyword, Number, String, Logical, Sign, Comment };
        State currentState = State::Search;

        std::optional<LexerError> error;
        size_t x = 1u, y = 1u, startX, startY;

        std::vector<Token> tokens;
        std::string target;
        size_t idx = 0, bidx = 0;
        bool wasEscaped = false, varMode = false, pardonLp = false, pardonPath = false;
    };

    static constexpr char FUNNEL_END_CHAR = '\0';
    static constexpr char COMMENT_CHAR = '#';
    static constexpr std::string_view WHITESPACE = " \t";
    static constexpr std::string_view SUB_ALLOWANCE_SYMBOLS = "-$( \t";

    static bool IsKeywordChar(char);

    void pattern(char, char, Context&);

    void searchPattern(char, char, Context&);
    void keywordPattern(char, Context&);
    void numberPattern(char, Context&);
    void stringPattern(char, Context&);
    void logicalPattern(char, Context&);
    void signPattern(char, Context&);
    void commentPattern(char, Context&);

    void resetState(Context&);
    void resetStart(Context&);
};

#endif
