#pragma once

#include <vector>
#include "baseClasses.hpp"
#include "functionRegistry.hpp"

class Evaluator {
    public:
        double evaluate(std::vector<Token> const& rpn);
};