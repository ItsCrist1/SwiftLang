#ifndef SHELLANG_LEXER_TEST_SUITE_H
#define SHELLANG_LEXER_TEST_SUITE_H
#include <gtest/gtest.h>
#include <string>
#include <tuple>

#include "Lexer.h"

using LexerTestCase = std::tuple<std::string, std::string, LexerOutput>;

class LexerTest : public ::testing::TestWithParam<LexerTestCase> {};

struct LexerTester {
    LexerTester(const Lexer&);
    void AssertTest(const LexerTestCase&);

private:
    Lexer lexer;

    void assertTokens(const std::string&, const std::vector<Token>&);
    void assertError(const std::string&, const LexerError&);

    void compareTokens(const std::vector<Token>&, const std::vector<Token>&);
};

#endif
