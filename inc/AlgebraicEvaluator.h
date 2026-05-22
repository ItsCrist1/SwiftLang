#ifndef SHELLANG_ALGEBRAICEVALUATOR_H
#define SHELLANG_ALGEBRAICEVALUATOR_H

#include "Context.h"
#include "Node.h"

#include <stack>
#include <functional>

struct Evaluator;

using AlgebraicEvaluatorOutput = std::variant<double,std::string>;

struct AlgebraicEvaluator {
    Context& context;

    explicit AlgebraicEvaluator(Context&);

    AlgebraicEvaluatorOutput Evaluate(const AlgebraicNode&);

    void setEvaluator(Evaluator*);

private:
    const int POW_PRECEDENCE = 2;

    enum class AlgebraicEvaluationCase { AllNum, AllStr, Halfsies };

    const std::unordered_map<AlgebraicOperator,int> OPERATOR_PRECEDENCES = {
        { AlgebraicOperator::Add, 0},
        { AlgebraicOperator::Sub, 0},
        { AlgebraicOperator::Mul, 1},
        { AlgebraicOperator::Div, 1},
        { AlgebraicOperator::Mod, 1},
        { AlgebraicOperator::Pow, POW_PRECEDENCE},
    };

    const std::unordered_map<LogicalOperator,int> LOGICAL_PRECEDENCES = {
        { LogicalOperator::Or, -3},
        { LogicalOperator::And, -2},
        { LogicalOperator::Equals, -1},
        { LogicalOperator::NotEquals, -1},
        { LogicalOperator::Lesser, -1},
        { LogicalOperator::LesserEquals, -1},
        { LogicalOperator::Greater, -1},
        { LogicalOperator::GreaterEquals, -1},
    };

    Evaluator* evaluator = nullptr;

    void evaluateToken(std::stack<AlgebraicEvaluatorOutput>&, const Token&);

    AlgebraicNode rearrange(const AlgebraicNode&);
    void rearrangeToken(bool&, bool&, AlgebraicNode&, std::stack<Token>&, const Token&, const std::function<void()>&);
    void rearrangeNode(bool&, AlgebraicNode&, const Node&, const std::function<void()>&);

    int getPrecedence(const Token&);
    std::string repeatStr(const std::string&, size_t);
};

#endif