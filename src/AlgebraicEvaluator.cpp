#include "AlgebraicEvaluator.h"

#include <stack>
#include <cmath>

AlgebraicEvaluator::AlgebraicEvaluator(Context& context) : context(context) {}

double AlgebraicEvaluator::Evaluate(const AlgebraicNode &an) {
    const AlgebraicNode tan = rearrange(an);

    std::stack<double> stack;

    for(const Token& token : tan.tokens) {
        if(std::holds_alternative<NumericToken>(token.value)) {
            stack.push(std::get<NumericToken>(token.value).value);
            continue;
        }

        if(std::holds_alternative<AlgebraicOperatorToken>(token.value)) {
            const double num2 = stack.top();
            stack.pop();
            const double num1 = stack.top();
            stack.pop();

            switch(std::get<AlgebraicOperatorToken>(token.value).op) {
                case AlgebraicOperator::Add: stack.push(num1 + num2); break;
                case AlgebraicOperator::Sub: stack.push(num1 - num2); break;
                case AlgebraicOperator::Mul: stack.push(num1 * num2); break;
                case AlgebraicOperator::Div: stack.push(num1 / num2); break;
                case AlgebraicOperator::Mod: stack.push(std::fmod(num1, num2)); break;
                case AlgebraicOperator::Pow: stack.push(std::pow(num1, num2)); break;
                default: break;
            }
        }
    }

    return stack.top();
}

AlgebraicNode AlgebraicEvaluator::rearrange(const AlgebraicNode& an) {
    AlgebraicNode output;
    std::stack<Token> stack;

    for(const Token& t : an.tokens) {
        if(std::holds_alternative<NumericToken>(t.value)) {
            output.tokens.push_back(t);
            continue;
        }

        if(std::holds_alternative<AlgebraicOperatorToken>(t.value)) {
            const auto aot = std::get<AlgebraicOperatorToken>(t.value);

            while(!stack.empty() && !std::holds_alternative<ParenthesesToken>(stack.top().value)
                && (PRECEDENCES.at(aot.op)
                < PRECEDENCES.at(std::get<AlgebraicOperatorToken>(stack.top().value).op)
            || PRECEDENCES.at(aot.op)
                == PRECEDENCES.at(std::get<AlgebraicOperatorToken>(stack.top().value).op)
                && PRECEDENCES.at(aot.op) < POW_PRECEDENCE)) {
                output.tokens.push_back(stack.top());
                stack.pop();
            }

            stack.emplace(t);
        }

        if(std::holds_alternative<VariableToken>(t.value)) {
            output.tokens.emplace_back(NumericToken(std::stod(context.Variables.at(std::get<VariableToken>(t.value).name))));
            continue;
        }

        if(std::holds_alternative<ParenthesesToken>(t.value)) {
            if(std::get<ParenthesesToken>(t.value).value == Parentheses::FuncOpen) {
                stack.push(t);
            } else {
                while(!std::holds_alternative<ParenthesesToken>(stack.top().value)) {
                    output.tokens.push_back(stack.top());
                    stack.pop();
                }
            }
        }
    }

    while(!stack.empty()) {
        output.tokens.push_back(stack.top());
        stack.pop();
    }

    return output;
}
