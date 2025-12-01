#include <iostream>
#include "parser.hpp"
#include "evaluator.hpp"
#include "functionRegistry.hpp"
#include <string>

class Calculator {
    private:
        Parser parser;
        Evaluator evaluator;
    
    public:
        Calculator() { FunctionRegistry::getInstance().loadPluginsFromDirectory(); }
        void run();
};