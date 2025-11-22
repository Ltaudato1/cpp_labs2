#include <iostream>
#include "calculator.hpp"

void Calculator::run() {
    std::cout << "Enter expressions to evaluate or type 'exit' to quit." << std::endl;
    std::string input;
    while (getline(std::cin, input)) {
        if (input.empty())
            continue;

        if (input == "exit" || input == "quit")
            break;

        try
        {
            std::cout << 
            evaluator.evaluate(
                parser.parse(input)
            ) 
            << std::endl;
        }
        catch(std::exception const& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}