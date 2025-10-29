#include <iostream>
#include "evaluator.hpp"
#include <stack>

double Evaluator::evaluate(const std::vector<Token>& rpnTokens) {
    std::stack<double> stack;
    
    for (const auto& token : rpnTokens) {
        if (token.type == NUMBER) {
            stack.push(std::stod(token.value));
        }
        else if (token.type == OPERATION) {
            if (!FunctionRegistry::getInstance().hasFunction(token.value)) {
                throw std::runtime_error("Unknown operation: " + token.value);
            }
            
            size_t arity = FunctionRegistry::getInstance().getFunctionArity(token.value);
            
            if (stack.size() < arity) {
                throw std::runtime_error("Not enough operands for operation: " + token.value);
            }
            
            std::vector<double> args;
            for (size_t i = 0; i < arity; ++i) {
                args.insert(args.begin(), stack.top());
                stack.pop();
            }
            
            double result = FunctionRegistry::getInstance().callFunction(token.value, args);
            stack.push(result);
        }
    }
    
    if (stack.size() != 1) {
        throw std::runtime_error("Invalid expression");
    }
    
    return stack.top();
}