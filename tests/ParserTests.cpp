#include "ParserTestSuite.h"
#include "Lexer.h"

#include <memory>

static ParserTester tester{Parser{}};

static std::vector<Token> lex(const std::string& src) {
    return std::get<std::vector<Token>>(Lexer{}.Lex(src));
}

static Node N(NodeValue v) { return Node{ std::move(v), 0, 0 }; }
static Token Tk(TokenValue v) { return Token{ std::move(v), 0, 0 }; }
static std::shared_ptr<Node> P(NodeValue v) { return std::make_shared<Node>(N(std::move(v))); }

TEST_P(ParserTest, ParseTest) {
    tester.AssertTest(GetParam());
}

INSTANTIATE_TEST_SUITE_P (
    ParserTests,
    ParserTest,
    ::testing::Values(
        // ---- Commands ----
        ParserTestCase{ "SingleCommand", lex("echo"),
            RootNode{{ N(CmdNode{"echo", {}}) }} },
        ParserTestCase{ "CommandWithArg", lex("echo hi"),
            RootNode{{ N(CmdNode{"echo", { N(ArgNode{"hi"}) }}) }} },
        ParserTestCase{ "CommandWithMultipleArgs", lex("echo hi there"),
            RootNode{{ N(CmdNode{"echo", { N(ArgNode{"hi"}), N(ArgNode{"there"}) }}) }} },
        ParserTestCase{ "CommandWithVariableArg", lex("echo $x"),
            RootNode{{ N(CmdNode{"echo", { N(VarNode{"x"}) }}) }} },
        ParserTestCase{ "StringSplitIntoArgs", lex("echo \"a b c\""),
            RootNode{{ N(CmdNode{"echo", { N(ArgNode{"a"}), N(ArgNode{"b"}), N(ArgNode{"c"}) }}) }} },

        // ---- Bare leaf nodes ----
        ParserTestCase{ "BareString", lex("\"hello\""),
            RootNode{{ N(StringNode{"hello"}) }} },
        ParserTestCase{ "BareVariable", lex("$x"),
            RootNode{{ N(VarNode{"x"}) }} },
        ParserTestCase{ "ArrayIndex", lex("$arr[0]"),
            RootNode{{ N(ArrNode{"arr", AlgebraicNode{{ Tk(NumericToken{0.0}) }}}) }} },

        // ---- Algebraic expressions ----
        ParserTestCase{ "SingleNumber", lex("5"),
            RootNode{{ N(AlgebraicNode{{ Tk(NumericToken{5.0}) }}) }} },
        ParserTestCase{ "SimpleArithmetic", lex("1 + 2"),
            RootNode{{ N(AlgebraicNode{{ Tk(NumericToken{1.0}),
                                        Tk(AlgebraicOperatorToken{AlgebraicOperator::Add}),
                                        Tk(NumericToken{2.0}) }}) }} },
        ParserTestCase{ "ArithmeticIsFlatTokenStream", lex("1 + 2 * 3"),
            RootNode{{ N(AlgebraicNode{{ Tk(NumericToken{1.0}),
                                        Tk(AlgebraicOperatorToken{AlgebraicOperator::Add}),
                                        Tk(NumericToken{2.0}),
                                        Tk(AlgebraicOperatorToken{AlgebraicOperator::Mul}),
                                        Tk(NumericToken{3.0}) }}) }} },
        ParserTestCase{ "VariableInArithmetic", lex("$x + 1"),
            RootNode{{ N(AlgebraicNode{{ N(VarNode{"x"}),
                                        Tk(AlgebraicOperatorToken{AlgebraicOperator::Add}),
                                        Tk(NumericToken{1.0}) }}) }} },
        ParserTestCase{ "DivisionExpression", lex("$total / 2"),
            RootNode{{ N(AlgebraicNode{{ N(VarNode{"total"}),
                                        Tk(AlgebraicOperatorToken{AlgebraicOperator::Div}),
                                        Tk(NumericToken{2.0}) }}) }} },
        ParserTestCase{ "ParenthesizedArithmetic", lex("(1 + 2) * 3"),
            RootNode{{ N(AlgebraicNode{{ Tk(ParenthesesToken{Parentheses::FuncOpen}),
                                        Tk(NumericToken{1.0}),
                                        Tk(AlgebraicOperatorToken{AlgebraicOperator::Add}),
                                        Tk(NumericToken{2.0}),
                                        Tk(ParenthesesToken{Parentheses::FuncClose}),
                                        Tk(AlgebraicOperatorToken{AlgebraicOperator::Mul}),
                                        Tk(NumericToken{3.0}) }}) }} },

        // ---- Redirects ----
        ParserTestCase{ "RedirectRight", lex("echo -> out.txt"),
            RootNode{{ N(RedirectNode{ P(CmdNode{"echo", {}}),
                                       P(CmdNode{"out.txt", {}}), Sign::RedirectRight }) }} },
        ParserTestCase{ "RedirectLeft", lex("cat <- in.txt"),
            RootNode{{ N(RedirectNode{ P(CmdNode{"cat", {}}),
                                       P(CmdNode{"in.txt", {}}), Sign::RedirectLeft }) }} },
        ParserTestCase{ "AppendRedirect", lex("echo hi >> log.txt"),
            RootNode{{ N(RedirectNode{ P(CmdNode{"echo", { N(ArgNode{"hi"}) }}),
                                       P(CmdNode{"log.txt", {}}), Sign::AppendRight }) }} },
        ParserTestCase{ "RedirectWithVariableArg", lex("echo $name -> out.txt"),
            RootNode{{ N(RedirectNode{ P(CmdNode{"echo", { N(VarNode{"name"}) }}),
                                       P(CmdNode{"out.txt", {}}), Sign::RedirectRight }) }} },
        ParserTestCase{ "VariableReadRedirect", lex("$i <- 2"),
            RootNode{{ N(RedirectNode{ P(VarNode{"i"}),
                                       P(AlgebraicNode{{ Tk(NumericToken{2.0}) }}),
                                       Sign::RedirectLeft }) }} },

        // ---- Control flow ----
        ParserTestCase{ "IfBlock", lex("if ($x) { echo hi }"),
            RootNode{{ N(IfNode{
                AlgebraicNode{{ Tk(ParenthesesToken{Parentheses::FuncOpen}),
                                N(VarNode{"x"}),
                                Tk(ParenthesesToken{Parentheses::FuncClose}) }},
                RootNode{{ N(CmdNode{"echo", { N(ArgNode{"hi"}) }}) }},
                RootNode{{}} }) }} },
        ParserTestCase{ "IfElseBlock", lex("if ($x) { echo hi } else { echo bye }"),
            RootNode{{ N(IfNode{
                AlgebraicNode{{ Tk(ParenthesesToken{Parentheses::FuncOpen}),
                                N(VarNode{"x"}),
                                Tk(ParenthesesToken{Parentheses::FuncClose}) }},
                RootNode{{ N(CmdNode{"echo", { N(ArgNode{"hi"}) }}) }},
                RootNode{{ N(CmdNode{"echo", { N(ArgNode{"bye"}) }}) }} }) }} },
        ParserTestCase{ "WhileBlock", lex("while ($i < 10) { echo $i }"),
            RootNode{{ N(WhileNode{
                AlgebraicNode{{ Tk(ParenthesesToken{Parentheses::FuncOpen}),
                                N(VarNode{"i"}),
                                Tk(LogicalOperatorToken{LogicalOperator::Lesser}),
                                Tk(NumericToken{10.0}),
                                Tk(ParenthesesToken{Parentheses::FuncClose}) }},
                RootNode{{ N(CmdNode{"echo", { N(VarNode{"i"}) }}) }} }) }} },

        ParserTestCase{ "ForBlock", lex("for($i <- 0; $i < 50; $i <- $i+1) { cp test }"),
            RootNode{{ N(ForNode{
                RedirectNode{ P(VarNode{"i"}),
                              P(AlgebraicNode{{ Tk(NumericToken{0.0}) }}),
                              Sign::RedirectLeft },
                RedirectNode{ P(VarNode{"i"}),
                              P(AlgebraicNode{{ Tk(ParenthesesToken{Parentheses::FuncOpen}),
                                                N(VarNode{"i"}),
                                                Tk(AlgebraicOperatorToken{AlgebraicOperator::Add}),
                                                Tk(NumericToken{1.0}),
                                                Tk(ParenthesesToken{Parentheses::FuncClose}) }}),
                              Sign::RedirectLeft },
                AlgebraicNode{{ N(VarNode{"i"}),
                                Tk(LogicalOperatorToken{LogicalOperator::Lesser}),
                                Tk(NumericToken{50.0}) }},
                RootNode{{ N(CmdNode{"cp", { N(ArgNode{"test"}) }}) }} }) }} },
        ParserTestCase{ "ForBlockLesserEqualsAndEcho", lex("for($n <- 1; $n <= 10; $n <- $n+2) { echo $n }"),
            RootNode{{ N(ForNode{
                RedirectNode{ P(VarNode{"n"}),
                              P(AlgebraicNode{{ Tk(NumericToken{1.0}) }}),
                              Sign::RedirectLeft },
                RedirectNode{ P(VarNode{"n"}),
                              P(AlgebraicNode{{ Tk(ParenthesesToken{Parentheses::FuncOpen}),
                                                N(VarNode{"n"}),
                                                Tk(AlgebraicOperatorToken{AlgebraicOperator::Add}),
                                                Tk(NumericToken{2.0}),
                                                Tk(ParenthesesToken{Parentheses::FuncClose}) }}),
                              Sign::RedirectLeft },
                AlgebraicNode{{ N(VarNode{"n"}),
                                Tk(LogicalOperatorToken{LogicalOperator::LesserEquals}),
                                Tk(NumericToken{10.0}) }},
                RootNode{{ N(CmdNode{"echo", { N(VarNode{"n"}) }}) }} }) }} },

        // ---- Multiple statements ----
        ParserTestCase{ "MultipleStatements", lex("echo a\necho b"),
            RootNode{{ N(CmdNode{"echo", { N(ArgNode{"a"}) }}),
                       N(CmdNode{"echo", { N(ArgNode{"b"}) }}) }} },

        ParserTestCase{ "LogicalAfterVariableSplitsNodes", lex("$x == 5 && echo yes"),
            RootNode{{ N(VarNode{"x"}),
                       N(AlgebraicNode{{ Tk(NumericToken{5.0}),
                                        Tk(LogicalOperatorToken{LogicalOperator::And}),
                                        N(CmdNode{"echo", { N(ArgNode{"yes"}) }}) }}) }} }
    ),
    [](const testing::TestParamInfo<ParserTest::ParamType>& param) {
        return std::get<0>(param.param);
    }
);
