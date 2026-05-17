#include "AlgebraicEvaluator.h"

#include <cmath>
#include "Evaluator.h"

AlgebraicEvaluator::AlgebraicEvaluator(Context& context) : context(context) {}

double AlgebraicEvaluator::Evaluate(const AlgebraicNode &an) {
    const AlgebraicNode tan = rearrange(an);

    std::stack<double> stack;

    for(const std::variant<Token,Node>& tn : tan.tns) {
        if(std::holds_alternative<Token>(tn))
            evaluateToken(stack, std::get<Token>(tn));
    }

    return stack.top();
}

void AlgebraicEvaluator::setEvaluator(Evaluator* evaluator) {
    this->evaluator = evaluator;
}

void AlgebraicEvaluator::evaluateToken(std::stack<double>& stack, const Token& t) {
    if(std::holds_alternative<NumericToken>(t.value)) {
        stack.push(std::get<NumericToken>(t.value).value);
        return;
    }

    if(std::holds_alternative<AlgebraicOperatorToken>(t.value)) {
        const double num2 = stack.top();
        stack.pop();
        const double num1 = stack.top();
        stack.pop();

        switch(std::get<AlgebraicOperatorToken>(t.value).op) {
            case AlgebraicOperator::Add: stack.push(num1 + num2); break;
            case AlgebraicOperator::Sub: stack.push(num1 - num2); break;
            case AlgebraicOperator::Mul: stack.push(num1 * num2); break;
            case AlgebraicOperator::Div: stack.push(num1 / num2); break;
            case AlgebraicOperator::Mod: stack.push(std::fmod(num1, num2)); break;
            case AlgebraicOperator::Pow: stack.push(std::pow(num1, num2)); break;
            default: break;
        }
    }

    if(std::holds_alternative<LogicalOperatorToken>(t.value)) {
        const double num2 = stack.top();
        stack.pop();
        const double num1 = stack.top();
        stack.pop();

        switch(std::get<LogicalOperatorToken>(t.value).op) {
            case LogicalOperator::And: stack.push(num1 && num2); break;
            case LogicalOperator::Or: stack.push(num1 || num2); break;
            case LogicalOperator::Equals: stack.push(num1 == num2); break;
            case LogicalOperator::NotEquals: stack.push(num1 != num2); break;
            case LogicalOperator::Lesser: stack.push(num1 < num2); break;
            case LogicalOperator::LesserEquals: stack.push(num1 <= num2); break;
            case LogicalOperator::Greater: stack.push(num1 > num2); break;
            case LogicalOperator::GreaterEquals: stack.push(num1 >= num2); break;
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
        output.tns.emplace_back(Token{NumericToken(std::stod(context.Variables.at(std::get<VariableToken>(t.value).name)))});
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

    wasOp = std::holds_alternative<AlgebraicOperatorToken>(t.value);
}

void AlgebraicEvaluator::rearrangeNode(bool& wasOp, AlgebraicNode& output, const Node& n, const std::function<void()>& flushNeg) {
    if(std::holds_alternative<CmdNode>(n.value)) {
        std::ostringstream oss;
        int rc = 0;

        evaluator->RunCmd(std::get<CmdNode>(n.value), oss, rc);

        output.tns.emplace_back(Token{NumericToken(std::stod(oss.str()))});

        flushNeg();
        wasOp = false;
    }
}

int AlgebraicEvaluator::getPrecedence(const Token& t) {
    if(std::holds_alternative<AlgebraicOperatorToken>(t.value))
        return OPERATOR_PRECEDENCES.at(std::get<AlgebraicOperatorToken>(t.value).op);

    if(std::holds_alternative<LogicalOperatorToken>(t.value))
        return LOGICAL_PRECEDENCES.at(std::get<LogicalOperatorToken>(t.value).op);

    return 0;
}
