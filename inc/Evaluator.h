#ifndef SHELLANG_EVALUATOR_H
#define SHELLANG_EVALUATOR_H

#include <sstream>

#include "AlgebraicEvaluator.h"
#include "Context.h"
#include "Error.h"
#include "Node.h"

using EvaluatorOutput = std::variant<int, EvaluatorError>;

struct Evaluator {
    explicit Evaluator(Context&);

    EvaluatorOutput Evaluate(const RootNode&, std::ostream* os=nullptr);
    bool RunCmd(const CmdNode&, std::ostream&, int&);

private:
    Context& context;
    AlgebraicEvaluator calculator;

    template<typename ... Ts>
    [[nodiscard]] bool is(const Node&) const;

    template<typename T>
    const T& as(const Node&) const;

    bool processCmd(const CmdNode&, std::ostream&, int&);
    void processRedirect(const RedirectNode&, std::ostream&, const std::vector<Node>&, int&);
    EvaluatorOutput processIf(const IfNode&, std::ostream&);
    EvaluatorOutput processWhile(const WhileNode&, std::ostream&);

    [[nodiscard]] const std::string& getVar(const std::string&) const;
    void setVar(const std::string&, const std::string&);
    [[nodiscard]] size_t getArrayIdx(const AlgebraicNode&);

    bool getConditionFromAlgebraicEvaluatorOutput(const AlgebraicEvaluatorOutput&);
};

#endif
