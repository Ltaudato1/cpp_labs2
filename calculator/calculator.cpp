#include <iostream>
#include "calculator.hpp"

void Calculator::run() {
    std::string input;
    while (getline(std::cin, input)) {
        std::cout << 
        evaluator.evaluate(
            parser.parse(input)
        ) 
        << std::endl;
    }
}