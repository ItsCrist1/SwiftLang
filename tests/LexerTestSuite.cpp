#include "LexerTestSuite.h"

LexerTester::LexerTester(const Lexer& lexer) : lexer(lexer) {}

void LexerTester::AssertTest(const LexerTestCase& tc) {
    const std::string& input = std::get<1>(tc);
    const LexerOutput output = std::get<2>(tc);

    if(std::holds_alternative<std::vector<Token>>(output))
        assertTokens(input, std::get<std::vector<Token>>(output));
    else
        assertError(input, std::get<LexerError>(output));
}

void LexerTester::assertTokens(const std::string& input, const std::vector<Token>& output) {
    const LexerOutput susOutput = lexer.Lex(input);

    ASSERT_TRUE(std::holds_alternative<std::vector<Token>>(susOutput)) << "Expected tokens, got error: " << std::get<LexerError>(susOutput).GetError();

    const std::vector<Token> clearOutput = std::get<std::vector<Token>>(susOutput);

    compareTokens(clearOutput, output);
}

void LexerTester::assertError(const std::string& input, const LexerError& output) {
    const LexerOutput susOutput = lexer.Lex(input);

    ASSERT_TRUE(std::holds_alternative<LexerError>(susOutput)) << "Expected error, got tokens";

    const LexerError clearOutput = std::get<LexerError>(susOutput);

    EXPECT_EQ(clearOutput.GetError(), output.GetError()) << "Different errors";
}

void LexerTester::compareTokens(const std::vector<Token>& a, const std::vector<Token>& b) {
    ASSERT_EQ(a.size(), b.size()) << "Token count mismatch";

    for(size_t i=0; i < a.size(); ++i)
        EXPECT_EQ(a[i].value, b[i].value) << "Token mismatch at index " << i;
}
