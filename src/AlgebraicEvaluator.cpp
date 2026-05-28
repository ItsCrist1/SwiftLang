#include "AlgebraicEvaluator.h"

#include <cmath>
#include "Evaluator.h"

AlgebraicEvaluator::AlgebraicEvaluator(Context& context) : context(context) {}

AlgebraicEvaluatorOutput AlgebraicEvaluator::Evaluate(const AlgebraicNode &an) {
    const AlgebraicNode tan = rearrange(an);

    std::stack<AlgebraicEvaluatorOutput> stack;

    for(const std::variant<Token,Node>& tn : tan.tns) {
        if(std::holds_alternative<Token>(tn))
            evaluateToken(stack, std::get<Token>(tn));
    }

    return stack.top();
}

void AlgebraicEvaluator::setEvaluator(Evaluator* evaluator) {
    this->evaluator = evaluator;
}

void AlgebraicEvaluator::evaluateToken(std::stack<AlgebraicEvaluatorOutput>& stack, const Token& t) {
    if(std::holds_alternative<NumericToken>(t.value)) {
        stack.emplace(std::get<NumericToken>(t.value).value);
        return;
    }

    if(std::holds_alternative<KeywordToken>(t.value)) {
        stack.emplace(std::get<KeywordToken>(t.value).cmd);
        return;
    }

    AlgebraicEvaluatorOutput t2 = stack.top();
    stack.pop();
    AlgebraicEvaluatorOutput t1 = stack.top();
    stack.pop();

    AlgebraicEvaluationCase aec;
    bool halfsiesOrder;

    std::string s1, s2;
    double n1, n2;

    if(std::holds_alternative<double>(t1) && std::holds_alternative<double>(t2)) {
        aec = AlgebraicEvaluationCase::AllNum;
        n1 = std::get<double>(t1);
        n2 = std::get<double>(t2);
    }
    else if(std::holds_alternative<std::string>(t1) && std::holds_alternative<std::string>(t2)) {
        aec = AlgebraicEvaluationCase::AllStr;
        s1 = std::get<std::string>(t1);
        s2 = std::get<std::string>(t2);
    }
    else {
        aec = AlgebraicEvaluationCase::Halfsies;

        halfsiesOrder = std::holds_alternative<std::string>(t1);

        if(halfsiesOrder) {
            s1 = std::get<std::string>(t1);
            n2 = std::get<double>(t2);
        } else {
            n1 = std::get<double>(t1);
            s2 = std::get<std::string>(t2);
        }
    }

    if(std::holds_alternative<AlgebraicOperatorToken>(t.value)) {
        switch(std::get<AlgebraicOperatorToken>(t.value).op) {
            case AlgebraicOperator::Add:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(n1 + n2);
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(s1 + s2);
            else if(!halfsiesOrder)
                    try {
                        stack.emplace(n1 + std::stod(s2));
                    } catch(const std::exception&) {
                        stack.emplace(std::to_string(n1) + s2);
                    }
            else
                try {
                    stack.emplace(std::stod(s1) + n2);
                } catch(const std::exception&) {
                    stack.emplace(s1 + std::to_string(n2));
                }
            break;

            case AlgebraicOperator::Sub:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(n1 - n2);
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(s1.ends_with(s2) ? s1.substr(0, s1.size() - s2.size()) : s1);
            else if(!halfsiesOrder)
                try {
                    stack.emplace(n1 - std::stod(s2));
                } catch(const std::exception&) {
                    const std::string& str1 = std::to_string(n1);
                    stack.emplace(str1.ends_with(s2) ? str1.substr(0, s1.size() - s2.size()) : str1);
                }
            else
                try {
                    stack.emplace(std::stod(s1) - n2);
                } catch(const std::exception&) {
                    const std::string& str1 = std::to_string(n2);
                    stack.emplace(s1.ends_with(str1) ? s1.substr(0, s1.size() - str1.size()) : s1);
                }
            break;

            case AlgebraicOperator::Mul:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(n1 * n2);
            else if(aec == AlgebraicEvaluationCase::AllStr) {
                try {
                    stack.emplace(repeatStr(s1, std::stoi(s2)));
                } catch(const std::exception&) {
                    try {
                        stack.emplace(repeatStr(s2, std::stoi(s1)));
                    } catch(const std::exception&) {
                        stack.emplace(s1);
                    }
                }
            } else if(halfsiesOrder)
                stack.emplace(repeatStr(s1, n2));
            else
                stack.emplace(repeatStr(s2, n1));

            break;

            case AlgebraicOperator::Div:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(n1 / n2);
            else if(aec == AlgebraicEvaluationCase::AllStr)
                try {
                    s1.resize(s1.size() / std::stoi(s2));
                    stack.emplace(s1);
                } catch(const std::exception&) {
                    try {
                        stack.emplace(static_cast<double>(std::stoi(s1) / s2.size()));
                    } catch(const std::exception&) {
                        stack.emplace(static_cast<double>(s1.size() / s2.size()));
                    }
                }
            else if(halfsiesOrder) {
                s1.resize(s1.size() / n2);
                stack.emplace(s1);
            } else
                stack.emplace(n1 / s2.size());
            break;

            case AlgebraicOperator::Mod:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(std::fmod(n1,n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                try {
                    if(const size_t sz = std::stoi(s2); sz < s1.size())
                        stack.emplace(static_cast<double>(s1[sz]));
                    else
                        stack.emplace(s1);
                } catch(const std::exception&) {
                    try {
                        if(const size_t sz = std::stoi(s1); s2.size() < sz)
                            stack.emplace(static_cast<double>(std::to_string(sz)[s2.size()]));
                        else
                            stack.emplace(s1);
                    } catch(const std::exception&) {
                        if(s2.size() < s1.size())
                            stack.emplace(static_cast<double>(s1[s2.size()]));
                        else
                            stack.emplace(s1);
                    }
                }
            else if(halfsiesOrder) {
                if(n2 < s1.size())
                    stack.emplace(static_cast<double>(s1[n2]));
                else
                    stack.emplace(s1);
            } else
                stack.emplace(n1);
            break;

            case AlgebraicOperator::Pow:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(std::pow(n1, n2));
            else if(aec == AlgebraicEvaluationCase::AllStr || halfsiesOrder) {
                std::string res;
                res.resize(s1.size());
                for(size_t i = 0; i < s1.size(); i++)
                    res[i] = s1[i] ^ s2[i % s2.size()];
                stack.emplace(res);
            }
            else
                stack.emplace(s2);
            break;

            default: break;
        }
    }

    if(std::holds_alternative<LogicalOperatorToken>(t.value)) {
        switch(std::get<LogicalOperatorToken>(t.value).op) {
            case LogicalOperator::And:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(static_cast<double>(n1 && n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(static_cast<double>(!s1.empty() && !s2.empty()));
            else if(halfsiesOrder)
                stack.emplace(static_cast<double>(!s1.empty() && n2));
            else
                stack.emplace(static_cast<double>(!s2.empty() && n1));
            break;

            case LogicalOperator::Or:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(static_cast<double>(n1 || n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(static_cast<double>(!s1.empty() || !s2.empty()));
            else if(halfsiesOrder)
                stack.emplace(static_cast<double>(!s1.empty() || n2));
            else
                stack.emplace(static_cast<double>(!s2.empty() || n1));
            break;

            case LogicalOperator::Equals:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(static_cast<double>(n1 == n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(static_cast<double>(s1 == s2));
            else if(halfsiesOrder)
                try {
                    stack.emplace(static_cast<double>(std::stod(s1) == n2));
                } catch(const std::exception&) {
                    stack.emplace(static_cast<double>(s1.size() == n2));
                }
            else
                try {
                    stack.emplace(static_cast<double>(std::stod(s2) == n1));
                } catch(const std::exception&) {
                    stack.emplace(static_cast<double>(s2.size() == n1));
                }
            break;

            case LogicalOperator::NotEquals:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(static_cast<double>(n1 != n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(static_cast<double>(s1 != s2));
            else if(halfsiesOrder)
                try {
                    stack.emplace(static_cast<double>(std::stod(s1) != n2));
                } catch(const std::exception&) {
                    stack.emplace(static_cast<double>(s1.size() != n2));
                }
            else
                try {
                    stack.emplace(static_cast<double>(std::stod(s2) != n1));
                } catch(const std::exception&) {
                    stack.emplace(static_cast<double>(s2.size() != n1));
                }
            break;

            case LogicalOperator::Lesser:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(static_cast<double>(n1 < n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(static_cast<double>(s1.size() < s2.size()));
            else if(halfsiesOrder)
                stack.emplace(static_cast<double>(s1.size() < n2));
            else
                stack.emplace(static_cast<double>(n1 < s2.size()));
            break;

            case LogicalOperator::LesserEquals:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(static_cast<double>(n1 <= n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(static_cast<double>(s1.size() <= s2.size()));
            else if(halfsiesOrder)
                stack.emplace(static_cast<double>(s1.size() <= n2));
            else
                stack.emplace(static_cast<double>(n1 <= s2.size()));
            break;

            case LogicalOperator::Greater:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(static_cast<double>(n1 > n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(static_cast<double>(s1.size() > s2.size()));
            else if(halfsiesOrder)
                stack.emplace(static_cast<double>(s1.size() > n2));
            else
                stack.emplace(static_cast<double>(n1 > s2.size()));
            break;

            case LogicalOperator::GreaterEquals:
            if(aec == AlgebraicEvaluationCase::AllNum)
                stack.emplace(static_cast<double>(n1 >= n2));
            else if(aec == AlgebraicEvaluationCase::AllStr)
                stack.emplace(static_cast<double>(s1.size() >= s2.size()));
            else if(halfsiesOrder)
                stack.emplace(static_cast<double>(s1.size() >= n2));
            else
                stack.emplace(static_cast<double>(n1 >= s2.size()));
            break;

            default: break;
        }
    }
}

AlgebraicNode AlgebraicEvaluator::rearrange(const AlgebraicNode& an) {
    AlgebraicNode output;
    std::stack<Token> stack;
    bool wasOp = false, toAddSub = false;

    auto flushNeg = [&]() {
        if(toAddSub) {
            output.tns.emplace_back(Token{NumericToken(-1)});
            output.tns.emplace_back(Token{AlgebraicOperatorToken(AlgebraicOperator::Mul)});
            toAddSub = false;
        }
    };

    for(const std::variant<Token,Node>& tn : an.tns) {
        if(std::holds_alternative<Token>(tn))
            rearrangeToken(wasOp, toAddSub, output, stack, std::get<Token>(tn), flushNeg);
        else
            rearrangeNode(wasOp, output, std::get<Node>(tn), flushNeg);
    }

    flushNeg();

    while(!stack.empty()) {
        output.tns.emplace_back(stack.top());
        stack.pop();
    }

    return output;
}

void AlgebraicEvaluator::rearrangeToken(bool& wasOp, bool& toAddSub, AlgebraicNode& output, std::stack<Token>& stack, const Token& t, const std::function<void()>& flushNeg) {
    if((wasOp || output.tns.empty()) && std::holds_alternative<AlgebraicOperatorToken>(t.value)
    && std::get<AlgebraicOperatorToken>(t.value).op == AlgebraicOperator::Sub)
        toAddSub = !toAddSub;
    else if(std::holds_alternative<NumericToken>(t.value)) {
        output.tns.emplace_back(t);
        flushNeg();
    }
    else if(std::holds_alternative<AlgebraicOperatorToken>(t.value)
    || std::holds_alternative<LogicalOperatorToken>(t.value)) {
        while(!stack.empty() && !std::holds_alternative<ParenthesesToken>(stack.top().value)
        && (getPrecedence(t) < getPrecedence(stack.top())
            || (getPrecedence(t) == getPrecedence(stack.top())
                && getPrecedence(t) < POW_PRECEDENCE))) {
            output.tns.emplace_back(stack.top());
            stack.pop();
        }

        stack.emplace(t);
    } else if(std::holds_alternative<VariableToken>(t.value)) {
        const std::string& var = context.Variables.Get(std::get<VariableToken>(t.value).name);

        try {
            output.tns.emplace_back(Token{NumericToken(std::stod(var))});
        }
        catch(const std::exception&) {
            output.tns.emplace_back(Token{ KeywordToken(var)});
        }

        flushNeg();
    }
    else if(std::holds_alternative<ParenthesesToken>(t.value)) {
        if(std::get<ParenthesesToken>(t.value).value == Parentheses::FuncOpen)
            stack.push(t);
        else {
            while(!std::holds_alternative<ParenthesesToken>(stack.top().value)) {
                output.tns.emplace_back(stack.top());
                stack.pop();
            }

            stack.pop();
            flushNeg();
        }
    }
    else if(std::holds_alternative<KeywordToken>(t.value))
        stack.push(t);
    else if(std::holds_alternative<StringToken>(t.value))
        stack.push(Token{KeywordToken(std::get<StringToken>(t.value).value)});

    wasOp = std::holds_alternative<AlgebraicOperatorToken>(t.value);
}

void AlgebraicEvaluator::rearrangeNode(bool& wasOp, AlgebraicNode& output, const Node& n, const std::function<void()>& flushNeg) {
    if(std::holds_alternative<CmdNode>(n.value)) {
        std::ostringstream oss;
        int rc = 0;

        evaluator->RunCmd(std::get<CmdNode>(n.value), oss, rc);

        try {
            output.tns.emplace_back(Token{NumericToken(std::stod(oss.str()))});
        }
        catch(const std::exception&) {
            output.tns.emplace_back(Token{KeywordToken(oss.str())});
        }

        flushNeg();
        wasOp = false;
    } else if(std::holds_alternative<VarNode>(n.value)) {
        const std::string& var = context.Variables.Get(std::get<VarNode>(n.value).var);

        try {
            output.tns.emplace_back(Token{NumericToken(std::stod(var))});
        } catch(const std::exception&) {
            output.tns.emplace_back(Token{KeywordToken(var)});
        }

        flushNeg();
        wasOp = false;
    }
    else if(std::holds_alternative<ArrNode>(n.value)) {
        const ArrNode an = std::get<ArrNode>(n.value);

        if(an.idx.tns.empty()) {
            output.tns.emplace_back(Token{NumericToken(
                static_cast<double>(context.Arrays.Get(an.arr).size())
            )});

            return;
        }


        const AlgebraicEvaluatorOutput aeo = Evaluate(an.idx);
        size_t idx;

        if(std::holds_alternative<double>(aeo))
            idx = std::get<double>(aeo);
        else
            try {
                idx = std::stoi(std::get<std::string>(aeo));
            } catch(const std::exception&) {
                idx = 0u;
            }

        const std::string str = context.Arrays.Get(an.arr)[idx];

        if(context.Arrays.Get(an.arr).size() > idx)
            try {
                output.tns.emplace_back(Token{NumericToken(std::stod(str))});
            } catch(const std::exception&) {
                output.tns.emplace_back(Token{KeywordToken(str)});
            }
    }
}

int AlgebraicEvaluator::getPrecedence(const Token& t) {
    if(std::holds_alternative<AlgebraicOperatorToken>(t.value))
        return OPERATOR_PRECEDENCES.at(std::get<AlgebraicOperatorToken>(t.value).op);

    if(std::holds_alternative<LogicalOperatorToken>(t.value))
        return LOGICAL_PRECEDENCES.at(std::get<LogicalOperatorToken>(t.value).op);

    return 0;
}

std::string AlgebraicEvaluator::repeatStr(const std::string& str, size_t c) {
    std::ostringstream oss;

    for(size_t i=0; i < c; ++i)
        oss << str;

    return oss.str();
}
