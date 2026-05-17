#include "Lexer.h"
#include <cctype>
#include <format>
#include <charconv>

Lexer::Lexer() = default;

LexerOutput Lexer::Lex(const std::string_view expression) {
    Context context;

    for(; context.idx < expression.length(); ++context.idx, ++context.x) {
        pattern(expression[context.idx], context.idx == expression.length()-1 ? FUNNEL_END_CHAR : expression[context.idx+1], context);

        if(context.error.has_value())
            return *context.error;
    }

    pattern(FUNNEL_END_CHAR, FUNNEL_END_CHAR, context);

    if(context.error.has_value())
        return *context.error;

    return context.tokens;
}

bool Lexer::IsKeywordChar(const char c) {
    return std::isalpha(c) || std::isdigit(c) || KeywordToken::KEYWORD_CHARS.contains(c);
}

void Lexer::pattern(const char c, const char cn, Context& context) {
    switch(context.currentState) {
        case Context::State::Search: searchPattern(c, cn, context); break;
        case Context::State::Keyword: keywordPattern(c, context); break;
        case Context::State::Number: numberPattern(c, context); break;
        case Context::State::String: stringPattern(c, context); break;
        case Context::State::Logical: logicalPattern(c, context); break;
        case Context::State::Sign: signPattern(c, context); break;
        case Context::State::Comment: commentPattern(c, context); break;
    }
}

void Lexer::searchPattern(const char c, const char cn, Context& context) {
    if(c == '-') {
        if(std::isdigit(cn) || SUB_ALLOWANCE_SYMBOLS.contains(cn))
            context.tokens.emplace_back(AlgebraicOperatorToken(AlgebraicOperator::Sub), context.x, context.y);
        else {
            context.currentState = Context::State::Keyword;
            context.target = c;
            resetStart(context);
        }

        return;
    }

    if(c == VariableToken::SIGN) {
        if(context.varMode)
            context.error = LexerError("Variable identifier found in variable mode", context.x, context.y);

        context.varMode = true;
        return;
    }

    if(std::isalpha(c) || KeywordToken::KEYWORD_CHARS.contains(c)) {
        context.currentState = Context::State::Keyword;
        context.target = c;
        resetStart(context);
        return;
    }

    if(std::isdigit(c)) {
        context.currentState = Context::State::Number;
        context.target = c;
        resetStart(context);
        return;
    }

    if(c == StringToken::QUOTE_CHAR) {
        context.currentState = Context::State::String;
        context.target = "";
        resetStart(context);
        return;
    }

    if(const auto it=ParenthesesToken::PARENTHESES.find(c); it != ParenthesesToken::PARENTHESES.end()) {
        context.tokens.emplace_back(ParenthesesToken(it->second), context.x, context.y);
        return;
    }

    if(const auto it=SParenthesesToken::SPARENTHESES.find(c); it != SParenthesesToken::SPARENTHESES.end()) {
        context.tokens.emplace_back(SParenthesesToken(it->second), context.x, context.y);
        return;
    }

    if(const auto it=AlgebraicOperatorToken::OPERATORS.find(c); it != AlgebraicOperatorToken::OPERATORS.end()) {
        context.tokens.emplace_back(AlgebraicOperatorToken(it->second), context.x, context.y);
        return;
    }

    if(!context.pardonLp && LogicalOperatorToken::ALL_LOGICAL_OPS.contains(c)) {
        context.currentState = Context::State::Logical;
        context.target = c;
        resetStart(context);
        context.bidx = context.idx;
        return;
    }

    if(SignToken::ALL_SIGNS.contains(c)) {
        context.currentState = Context::State::Sign;
        context.target = c;
        resetStart(context);
        context.pardonLp = false;
        return;
    }

    if(NewlineToken::NEWLINE_CHARS.contains(c)) {
        context.tokens.emplace_back(NewlineToken(), context.x, context.y);

        context.x = 0u;
        ++context.y;

        return;
    }

    if(c == COMMENT_CHAR) {
        context.currentState = Context::State::Comment;
        return;
    }

    if(WHITESPACE.contains(c))
        return;

    if(c != FUNNEL_END_CHAR)
        context.error = LexerError(std::format("Unknown character encountered: {}", c), context.x, context.y);
}

void Lexer::keywordPattern(const char c, Context& context) {
    if(IsKeywordChar(c)) {
        context.target.push_back(c);
        return;
    }

    if(context.varMode)
        context.tokens.emplace_back(VariableToken(context.target), context.startX, context.startY);
    else
        context.tokens.emplace_back(KeywordToken(context.target), context.startX, context.startY);

    resetState(context);
}

void Lexer::numberPattern(const char c, Context& context) {
    if(std::isdigit(c) || c == NumericToken::DECIMAL_CHAR) {
        context.target.push_back(c);
        return;
        
    }

    if(NumericToken::NUMBER_EXCEPTIONS.contains(c))
        return;

    if(context.target.back() == NumericToken::DECIMAL_CHAR) {
        context.error = LexerError(std::format("Incomplete number: {}.", context.target), context.startX, context.startY);
        return;
    }

    double d;
    std::from_chars(context.target.data(), context.target.data() + context.target.size(), d);

    context.tokens.emplace_back(NumericToken(d), context.startX, context.startY);
    resetState(context);
}

void Lexer::stringPattern(const char c, Context& context) {
    if(c == StringToken::QUOTE_CHAR) {
        if(context.wasEscaped) {
            context.wasEscaped = false;
            context.target.push_back(c);
            return;
        }

        context.tokens.emplace_back(StringToken(context.target), context.startX, context.startY);
        resetState(context);
        ++context.idx;
        return;
    }

    if(c == StringToken::ESCAPE_CHAR) {
        context.wasEscaped = true;
        return;
    }

    if(c == FUNNEL_END_CHAR) {
        context.error = LexerError(std::format("Incomplete string: {}", context.target), context.startX, context.startY);
        return;
    }

    context.target.push_back(c);
}

void Lexer::logicalPattern(const char c, Context& context) {
    if(LogicalOperatorToken::ALL_LOGICAL_OPS.contains(c)) {
        context.target.push_back(c);
        return;
    }

    if(const auto it = LogicalOperatorToken::LOGICAL_OPS.find(context.target); it != LogicalOperatorToken::LOGICAL_OPS.end()) {
        context.tokens.emplace_back(LogicalOperatorToken(it->second), context.startX, context.startY);
        resetState(context);
    } else {
        context.idx = context.bidx - 1;
        context.pardonLp = true;
        resetState(context);
    }
}

void Lexer::signPattern(const char c, Context& context) {
    if(SignToken::ALL_SIGNS.contains(c)) {
        context.target.push_back(c);
        return;
    }

    if(const auto it = SignToken::SIGNS.find(context.target); it != SignToken::SIGNS.end()) {
        context.tokens.emplace_back(SignToken(it->second), context.startX, context.startY);
        resetState(context);
    } else {
        context.error = LexerError(std::format("Invalid sign: {}", context.target), context.startX, context.startY);
        return;
    }
}

void Lexer::commentPattern(const char c, Context& context) {
    if(NewlineToken::NEWLINE_CHARS.contains(c))
        resetState(context);
}

void Lexer::resetState(Context& context) {
    context.currentState = Context::State::Search;
    --context.idx;
    --context.x;

    context.varMode = false;
}

void Lexer::resetStart(Context& context) {
    context.startX = context.x;
    context.startY = context.y;
}
