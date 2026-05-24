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

        if(is<StringNode>(node)) {
            usedOs << as<StringNode>(node).str;
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

        if(is<ArrNode>(node)) {
            const ArrNode an = as<ArrNode>(node);

            if(an.idx.tns.empty()) {
                usedOs << context.Arrays[an.arr].size();
                continue;
            }

            const size_t idx = getArrayIdx(an.idx);

            if(context.Arrays[an.arr].size() > idx)
                usedOs << context.Arrays[an.arr][idx];

            continue;
        }

        if(is<AlgebraicNode>(node)) {
            std::visit([&usedOs](const auto& v) { usedOs << v << '\n'; }, calculator.Evaluate(as<AlgebraicNode>(node)));
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

        if(is<ArrNode>(node)) {
            const ArrNode an = as<ArrNode>(node);

            if(an.idx.tns.empty()) {
                args.emplace_back(std::to_string(context.Arrays[an.arr].size()));
                continue;
            }

            const size_t idx = getArrayIdx(an.idx);
            if(context.Arrays[an.arr].size() > idx)
                args.emplace_back(context.Arrays[an.arr][idx]);
        }

        if(is<AlgebraicNode>(node)) {
            const AlgebraicEvaluatorOutput& aeo = calculator.Evaluate(as<AlgebraicNode>(node));
            args.emplace_back(std::holds_alternative<std::string>(aeo)
                ? std::get<std::string>(aeo)
                : std::to_string(std::get<double>(aeo)));
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
    }
    else if(is<StringNode>(*Source))
        source << as<StringNode>(*Source).str;
    else if(is<RedirectNode>(*Source))
        processRedirect(as<RedirectNode>(*Source), source, args, returnCode);
    else if(is<VarNode>(*Source))
        source << getVar(as<VarNode>(*Source).var);
    else if(is<ArrNode>(*Source)) {
        const ArrNode an = as<ArrNode>(*Source);

        if(an.idx.tns.empty())
            source << context.Arrays[an.arr].size();
        else {
            const size_t idx = getArrayIdx(an.idx);

            if(context.Arrays[an.arr].size() <= idx)
                context.Arrays[an.arr].resize(idx + 1);

            source << context.Arrays[an.arr][idx];
        }
    }
    else if(is<AlgebraicNode>(*Source))
        std::visit([&source](const auto& v) { source << v; }, calculator.Evaluate(as<AlgebraicNode>(*Source)));
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
    else if(is<ArrNode>(*Target)) {
        const ArrNode an = as<ArrNode>(*Target);

        if(an.idx.tns.empty()) {
            auto& arr = context.Arrays[an.arr];
            arr.resize(targs.size());
             std::transform(targs.begin(), targs.end(), arr.begin(), [this](const Node& an) {
                return as<ArgNode>(an).arg;
             });

            return;
        }

        const size_t idx = getArrayIdx(an.idx);

        if(context.Arrays[an.arr].size() <= idx)
            context.Arrays[an.arr].resize(idx + 1);

        context.Arrays[an.arr][idx] = source.str();
    }

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

    static const std::string empty;
    return empty;
}

void Evaluator::setVar(const std::string& var, const std::string& val) {
    if(const auto it=context.Variables.find(var); it != context.Variables.end())
        it->second = val;
    else
        context.Variables.emplace(var, val);
}

size_t Evaluator::getArrayIdx(const AlgebraicNode& an) {
    const AlgebraicEvaluatorOutput aeo = calculator.Evaluate(an);
    size_t idx;

    if(std::holds_alternative<double>(aeo))
        idx = std::get<double>(aeo);
    else
        try {
            idx = std::stoi(std::get<std::string>(aeo));
        } catch(const std::exception&) {
            idx = 0u;
        }

    return idx;
}

EvaluatorOutput Evaluator::processIf(const IfNode& in, std::ostream& os) {
    if(!getConditionFromAlgebraicEvaluatorOutput(calculator.Evaluate(in.condition)))
        return Evaluate(in.elseBody, &os);

    return Evaluate(in.ifBody, &os);
}

EvaluatorOutput Evaluator::processWhile(const WhileNode& wn, std::ostream& os) {
    while(getConditionFromAlgebraicEvaluatorOutput(calculator.Evaluate(wn.condition)))
        if(const auto result = Evaluate(wn.body, &os);
            std::holds_alternative<int>(result) && std::get<int>(result) != 0)
            return result;

    return 0;
}

bool Evaluator::getConditionFromAlgebraicEvaluatorOutput(const AlgebraicEvaluatorOutput& aeo) {
    if(std::holds_alternative<double>(aeo))
        return std::get<double>(aeo);

    if(std::holds_alternative<std::string>(aeo))
        return !std::get<std::string>(aeo).empty();
}
