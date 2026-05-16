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

    const std::unordered_map<AlgebraicOperator,int> PRECEDENCES = {
        { AlgebraicOperator::Add, 0},
        { AlgebraicOperator::Sub, 0},
        { AlgebraicOperator::Mul, 1},
        { AlgebraicOperator::Div, 1},
        { AlgebraicOperator::Mod, 1},
        { AlgebraicOperator::Pow, POW_PRECEDENCE},
    };

    AlgebraicNode rearrange(const AlgebraicNode&);
};

#endif