#ifndef SHELLANG_EVALUATOR_H
#define SHELLANG_EVALUATOR_H

#include <sstream>

#include "Context.h"
#include "Node.h"

struct Evaluator {
    explicit Evaluator(Context&);

    int Evaluate(const RootNode&);

private:
    Context& context;

    template<typename ... Ts>
    [[nodiscard]] bool is(const Node&) const;

    template<typename T>
    const T& as(const Node&) const;

    bool processCmd(const CmdNode&, std::ostream&);
    void processRedirect(const RedirectNode&, std::ostream&, const std::vector<Node>&);
};

#endif
