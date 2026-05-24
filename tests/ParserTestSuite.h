#ifndef SHELLANG_PARSERTESTSUITE_H
#define SHELLANG_PARSERTESTSUITE_H
#include <gtest/gtest.h>
#include <string>
#include <tuple>
#include <vector>

#include "Parser.h"

using ParserTestCase = std::tuple<std::string, std::vector<Token>, ParserOutput>;

class ParserTest : public ::testing::TestWithParam<ParserTestCase> {};

struct ParserTester {
    ParserTester(const Parser&);
    void AssertTest(const ParserTestCase&);

private:
    Parser parser;

    void assertNodes(const std::vector<Token>&, const RootNode&);
    void assertError(const std::vector<Token>&, const ParserError&);

    void compareNodes(const std::vector<Node>&, const std::vector<Node>&);
};

#endif