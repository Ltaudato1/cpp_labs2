#include <iostream>

#include "parser.hpp"
#include "baseClasses.hpp"
#include "evaluator.hpp"
#include "functionRegistry.hpp"
#include <string>

int main() {
    std::string expr;
    FunctionRegistry::getInstance().loadPluginsFromDirectory();
    std::getline(std::cin, expr);
    Parser parser;
    Evaluator evaluator;
    auto rpn = parser.parse(expr);
    std::cout << evaluator.evaluate(rpn) << std::endl;
    return 0;
}