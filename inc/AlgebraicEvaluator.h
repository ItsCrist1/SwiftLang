#ifndef SHELLANG_ALGEBRAICEVALUATOR_H
#define SHELLANG_ALGEBRAICEVALUATOR_H

#include "Context.h"
#include "Node.h"

struct AlgebraicEvaluator {
    Context& context;

    explicit AlgebraicEvaluator(Context&);

    double Evaluate(const AlgebraicNode&);
};

#endif