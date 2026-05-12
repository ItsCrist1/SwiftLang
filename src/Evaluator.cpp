#include "Evaluator.h"

#include <fstream>

Evaluator::Evaluator(Context& context) : context(context) {}

int Evaluator::Evaluate(const RootNode& rn) {
    for(const Node& node : rn.nodes) {
        if(is<CmdNode>(node)) {
            processCmd(as<CmdNode>(node), context.OutputStream);
            continue;
        }

        if(is<RedirectNode>(node))
            processRedirect(as<RedirectNode>(node), context.OutputStream);
    }

    return 0;
}

template<typename... Ts>
bool Evaluator::is(const Node& node) const {
    return (std::holds_alternative<Ts>(node.value) || ...);
}

template<typename T>
const T& Evaluator::as(const Node& node) const {
    return std::get<T>(node.value);
}

bool Evaluator::processCmd(const CmdNode& cmd, std::ostream& os) {
    std::vector<std::string> args;
    args.reserve(cmd.args.size());

    for(const Node& node : cmd.args) {
        if(is<ArgNode>(node)) {
            args.emplace_back(as<ArgNode>(node).arg);
            continue;
        }
    }

    if(const auto it=context.Commands.find(cmd.cmd); it != context.Commands.end()) {
        it->second->Run(args, context, context.InputStream, os);
        return true;
    }

    return false;
}

void Evaluator::processRedirect(const RedirectNode& redirect, std::ostream& os) {
    std::shared_ptr<Node> Source, Target;

    if(redirect.sign == Sign::RedirectLeft
    || redirect.sign == Sign::AppendLeft) {
        Source = redirect.SideLeft;
        Target = redirect.SideRight;
    } else {
        Source = redirect.SideRight;
        Target = redirect.SideLeft;
    }

    std::ostringstream source;
    bool isFile = false;

    if(is<CmdNode>(*Source)) {
        const CmdNode& cn = as<CmdNode>(*Source);

        isFile = !processCmd(cn, source);
        if(isFile) {
            source.str("");
            source << cn.cmd;
        }
    }
    else if(is<RedirectNode>(*Source))
        processRedirect(as<RedirectNode>(*Source), source);

    std::istringstream iss (source.str());
    std::vector<Node> args;

    for(std::string t; iss >> t;)
        args.emplace_back(ArgNode(t));

    std::ofstream ofs;

    if(isFile) {
        const bool append = redirect.sign == Sign::AppendLeft || redirect.sign == Sign::AppendRight;
        ofs.open(source.str(), append ? std::ios::app : std::ios::out);
    }

    if(is<CmdNode>(*Target)) {
        processCmd(CmdNode{as<CmdNode>(*Target).cmd, args}, isFile ? ofs : os);
        return;
    }

    if(is<RedirectNode>(*Target)) {
        processRedirect(as<RedirectNode>(*Target), isFile ? ofs : os);
        return;
    }

    ofs.close();
}
