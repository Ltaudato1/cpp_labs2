#pragma once

#include <vector>
#include "base_classes.hpp"
#include "function_registy.hpp"

class Evaluator {
    public:
        double evaluate(std::vector<Token> const& rpn, FunctionRegistry const& registry);
};
