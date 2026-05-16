#ifndef SHELLANG_EVALUATOR_H
#define SHELLANG_EVALUATOR_H

#include <sstream>

#include "Context.h"
#include "Error.h"
#include "Node.h"

using EvaluatorOutput = std::variant<int, EvaluatorError>;

struct Evaluator {
    explicit Evaluator(Context&);

    EvaluatorOutput Evaluate(const RootNode&);

private:
    Context& context;

    template<typename ... Ts>
    [[nodiscard]] bool is(const Node&) const;

    template<typename T>
    const T& as(const Node&) const;

    bool processCmd(const CmdNode&, std::ostream&, int&);
    void processRedirect(const RedirectNode&, std::ostream&, const std::vector<Node>&, int&);

    [[nodiscard]] std::string_view getVar(const std::string&) const;
    void setVar(const std::string&, const std::string&);
};

#endif
