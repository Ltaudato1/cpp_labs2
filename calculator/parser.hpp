#pragma once

#include <vector>
#include "base_classes.hpp"

class Parser {
public:
    std::vector<Token> Parse(std::string const& expr);
    std::vector<Token> ToRPN(std::vector<Token> const& tokens);
};