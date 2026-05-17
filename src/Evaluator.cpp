#include "Evaluator.h"

#include <fstream>

Evaluator::Evaluator(Context& context) : context(context), calculator(context) {
    calculator.setEvaluator(this);
}

EvaluatorOutput Evaluator::Evaluate(const RootNode& rn, std::ostream* os) {
    std::ostream& usedOs = os != nullptr ? *os : context.OutputStream;

    for(const Node& node : rn.nodes) {
        if(is<CmdNode>(node)) {
            int rc = 0;
            processCmd(as<CmdNode>(node), usedOs, rc);

            if(rc != 0)
                return rc;
            continue;
        }

        if(is<RedirectNode>(node)) {
            int rc = 0;
            processRedirect(as<RedirectNode>(node), usedOs, {}, rc);

            if(rc != 0)
                return rc;

            continue;
        }

        if(is<VarNode>(node)) {
            usedOs << getVar(as<VarNode>(node).var) << '\n';
            continue;
        }

        if(is<AlgebraicNode>(node)) {
            usedOs << calculator.Evaluate(as<AlgebraicNode>(node)) << '\n';
            continue;
        }

        if(is<IfNode>(node)) {
            if(const auto res = processIf(as<IfNode>(node), usedOs);
                std::holds_alternative<int>(res) && std::get<int>(res) != 0)
                return res;

            continue;
        }

        if(is<WhileNode>(node)) {
            if(const auto res = processWhile(as<WhileNode>(node), usedOs);
            std::holds_alternative<int>(res) && std::get<int>(res) != 0)
                return res;

            continue;
        }
    }

    return 0;
}

bool Evaluator::RunCmd(const CmdNode& cn, std::ostream& os, int& n) {
    return processCmd(cn, os, n);
}

template<typename... Ts>
bool Evaluator::is(const Node& node) const {
    return (std::holds_alternative<Ts>(node.value) || ...);
}

template<typename T>
const T& Evaluator::as(const Node& node) const {
    return std::get<T>(node.value);
}

bool Evaluator::processCmd(const CmdNode& cmd, std::ostream& os, int& returnCode) {
    std::vector<std::string> args;
    args.reserve(cmd.args.size());

    for(const Node& node : cmd.args) {
        if(is<ArgNode>(node)) {
            args.emplace_back(as<ArgNode>(node).arg);
            continue;
        }

        if(is<VarNode>(node)) {
            args.emplace_back(getVar(as<VarNode>(node).var));
            continue;
        }

        if(is<AlgebraicNode>(node)) {
            args.emplace_back(std::to_string(calculator.Evaluate(as<AlgebraicNode>(node))));
            continue;
        }
    }

    if(const auto it=context.Commands.find(cmd.cmd); it != context.Commands.end()) {
        returnCode = it->second->Run(args, context, context.InputStream, os);
        return true;
    }

    return false;
}

void Evaluator::processRedirect(const RedirectNode& redirect, std::ostream& os, const std::vector<Node>& args, int& returnCode) {
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
        if(!processCmd(CmdNode(cn.cmd, combined), source, returnCode)) {
            std::ifstream is (cn.cmd);
            source << is.rdbuf();
            is.close();
        }
    } else if(is<RedirectNode>(*Source))
        processRedirect(as<RedirectNode>(*Source), source, args, returnCode);
    else if(is<VarNode>(*Source))
        source << getVar(as<VarNode>(*Source).var);
    else if(is<AlgebraicNode>(*Source))
        source << calculator.Evaluate(as<AlgebraicNode>(*Source));
    else if(is<IfNode>(*Source))
        processIf(as<IfNode>(*Source), source);
    else if(is<WhileNode>(*Source))
        processWhile(as<WhileNode>(*Source), source);

    std::istringstream iss (source.str());
    std::vector<Node> targs;

    for(std::string t; iss >> t;)
        targs.emplace_back(ArgNode(t));

    bool isFile = false;
    std::string fileName;

    if(is<CmdNode>(*Target)) {
        const std::string& cns = as<CmdNode>(*Target).cmd;

        isFile = !processCmd(CmdNode{cns, targs}, os, returnCode);

        if(isFile)
            fileName = cns;
    }
    else if(is<RedirectNode>(*Target))
        processRedirect(as<RedirectNode>(*Target), os, targs, returnCode);
    else if(is<VarNode>(*Target))
        setVar(as<VarNode>(*Target).var, source.str());

    if(isFile) {
        const bool append = redirect.sign == Sign::AppendLeft || redirect.sign == Sign::AppendRight;
        std::ofstream ofs (fileName, append ? std::ios::app : std::ios::out);

        ofs << source.str();
        ofs.close();
    }
}

const std::string& Evaluator::getVar(const std::string& var) const {
    if(const auto it=context.Variables.find(var); it != context.Variables.end())
        return it->second;

    return "";
}

void Evaluator::setVar(const std::string& var, const std::string& val) {
    if(const auto it=context.Variables.find(var); it != context.Variables.end())
        it->second = val;
    else
        context.Variables.emplace(var, val);
}

EvaluatorOutput Evaluator::processIf(const IfNode& in, std::ostream& os) {
    if(!calculator.Evaluate(in.condition))
        return Evaluate(in.elseBody, &os);

    return Evaluate(in.ifBody, &os);
}

EvaluatorOutput Evaluator::processWhile(const WhileNode& wn, std::ostream& os) {
    while(calculator.Evaluate(wn.condition))
        if(const auto result = Evaluate(wn.body, &os);
            std::holds_alternative<int>(result) && std::get<int>(result) != 0)
            return result;

    return 0;
}