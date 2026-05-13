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
            processRedirect(as<RedirectNode>(node), context.OutputStream, {});
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

void Evaluator::processRedirect(const RedirectNode& redirect, std::ostream& os, const std::vector<Node>& args) {
    std::shared_ptr<Node> Source, Target;

    if(redirect.sign == Sign::RedirectLeft
    || redirect.sign == Sign::AppendLeft) {
        Source = redirect.SideRight;
        Target = redirect.SideLeft;
    } else {
        Source = redirect.SideLeft;
        Target = redirect.SideRight;
    }

    std::ostringstream source;

    if(is<CmdNode>(*Source)) {
        const auto& cn = as<CmdNode>(*Source);
        auto combined = cn.args;
        combined.insert(combined.end(), args.begin(), args.end());
        if(!processCmd(CmdNode(cn.cmd, combined), source)) {
            std::ifstream is (cn.cmd);
            source << is.rdbuf();
            is.close();
        }
    }
    else if(is<RedirectNode>(*Source))
        processRedirect(as<RedirectNode>(*Source), source, args);

    std::istringstream iss (source.str());
    std::vector<Node> targs;

    for(std::string t; iss >> t;)
        targs.emplace_back(ArgNode(t));

    bool isFile = false;
    std::string fileName;

    if(is<CmdNode>(*Target)) {
        const std::string& cns = as<CmdNode>(*Target).cmd;

        isFile = !processCmd(CmdNode{cns, targs}, os);

        if(isFile)
            fileName = cns;
    }
    else if(is<RedirectNode>(*Target))
        processRedirect(as<RedirectNode>(*Target), os, targs);

    if(isFile) {
        const bool append = redirect.sign == Sign::AppendLeft || redirect.sign == Sign::AppendRight;
        std::ofstream ofs (fileName, append ? std::ios::app : std::ios::out);

        ofs << source.str();
        ofs.close();
    }
}
