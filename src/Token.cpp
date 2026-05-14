#include "Token.h"

KeywordToken::KeywordToken(const std::string_view cmd) : cmd(cmd) {}
VariableToken::VariableToken(const std::string_view name) : name(name) {}
NumericToken::NumericToken(const double value) : value(value) {}

StringToken::StringToken(const std::string_view value) : value(value) {}

ParenthesesToken::ParenthesesToken(const Parentheses value) : value(value) {}

AlgebraicOperatorToken::AlgebraicOperatorToken(const AlgebraicOperator op) : op(op) {}
LogicalOperatorToken::LogicalOperatorToken(const LogicalOperator op) : op(op) {}

SignToken::SignToken(const Sign sign) : sign(sign) {}

NewlineToken::NewlineToken() = default;
