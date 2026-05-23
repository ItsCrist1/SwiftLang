#include "Token.h"

KeywordToken::KeywordToken(const std::string_view cmd) : cmd(cmd) {}
VariableToken::VariableToken(const std::string_view name) : name(name) {}
FuncToken::FuncToken(const std::string_view name) : name(name) {}
NumericToken::NumericToken(const double value) : value(value) {}

StringToken::StringToken(const std::string_view value) : value(value) {}

ParenthesesToken::ParenthesesToken(const Parentheses value) : value(value) {}
MiscParenthesesToken::MiscParenthesesToken(const MiscParentheses value) : value(value) {}
FuncParenthesesToken::FuncParenthesesToken(const FuncParentheses value) : value(value) {}

AlgebraicOperatorToken::AlgebraicOperatorToken(const AlgebraicOperator op) : op(op) {}
LogicalOperatorToken::LogicalOperatorToken(const LogicalOperator op) : op(op) {}

SignToken::SignToken(const Sign sign) : sign(sign) {}

NewlineToken::NewlineToken() = default;
