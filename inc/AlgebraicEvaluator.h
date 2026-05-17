#ifndef SHELLANG_ALGEBRAICEVALUATOR_H
#define SHELLANG_ALGEBRAICEVALUATOR_H

#include "Context.h"
#include "Node.h"

struct AlgebraicEvaluator {
    Context& context;

    explicit AlgebraicEvaluator(Context&);

    double Evaluate(const AlgebraicNode&);

private:
    const int POW_PRECEDENCE = 2;

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

    AlgebraicNode rearrange(const AlgebraicNode&);
    int getPrecedence(const Token&);
};

#endif