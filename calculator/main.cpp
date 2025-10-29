#include <iostream>

#include "parser.hpp"
#include "baseClasses.hpp"
#include <string>

int main() {
    std::string expr;
    std::getline(std::cin, expr);
    Parser parser;
    auto rpn = parser.parse(expr);
    for (auto token: rpn) {
        std::cout << token.value << " ";
    }
    std::cout << std::endl;
    return 0;
}