#include "ParserTestSuite.h"

ParserTester::ParserTester(const Parser& parser) : parser(parser) {}

void ParserTester::AssertTest(const ParserTestCase& tc) {
    const std::vector<Token>& input = std::get<1>(tc);
    const ParserOutput output = std::get<2>(tc);

    if(std::holds_alternative<RootNode>(output))
        assertNodes(input, std::get<RootNode>(output));
    else
        assertError(input, std::get<ParserError>(output));
}

void ParserTester::assertNodes(const std::vector<Token>& input, const RootNode& output) {
    const ParserOutput susOutput = parser.Parse(input);

    ASSERT_TRUE(std::holds_alternative<RootNode>(susOutput)) << "Expected nodes, got error: " << std::get<ParserError>(susOutput).GetError();

    const RootNode clearOutput = std::get<RootNode>(susOutput);

    compareNodes(clearOutput.nodes, output.nodes);
}

void ParserTester::assertError(const std::vector<Token>& input, const ParserError& output) {
    const ParserOutput susOutput = parser.Parse(input);

    ASSERT_TRUE(std::holds_alternative<ParserError>(susOutput)) << "Expected error, got nodes";

    const ParserError clearOutput = std::get<ParserError>(susOutput);

    EXPECT_EQ(clearOutput.GetError(), output.GetError()) << "Different errors";
}

void ParserTester::compareNodes(const std::vector<Node>& a, const std::vector<Node>& b) {
    ASSERT_EQ(a.size(), b.size()) << "Node count mismatch";
    for(size_t i=0; i < a.size(); ++i)
        EXPECT_EQ(a[i].value, b[i].value) << "Node mismatch at index " << i;
}