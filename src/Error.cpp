#include "Error.h"

#include <sstream>

Error::Error(const std::string_view ErrorMessage, const size_t x, const size_t y) : ErrorMessage(ErrorMessage), x(x), y(y) {}

LexerError::LexerError(const std::string_view ErrorMessage, const size_t x, const size_t y) : Error(ErrorMessage, x, y) {}

std::string LexerError::GetError() const {
    std::stringstream sstr;
    sstr << "[LEXER ERROR | Row: " << y << " | Column: " << x << "] " << ErrorMessage;
    return sstr.str();
}

ParserError::ParserError(const std::string_view ErrorMessage, const size_t x, const size_t y) : Error(ErrorMessage, x, y) {}

std::string ParserError::GetError() const {
    std::stringstream sstr;
    sstr << "[PARSER ERROR | Row: " << y << " | Column: " << x << "] " << ErrorMessage;
    return sstr.str();
}

EvaluatorError::EvaluatorError(const std::string_view ErrorMessage, const size_t x, const size_t y) : Error(ErrorMessage, x, y) {}

std::string EvaluatorError::GetError() const {
    std::stringstream sstr;
    sstr << "[EVALUATOR ERROR | Row: " << y << " | Column: " << x << "] " << ErrorMessage;
    return sstr.str();
}
