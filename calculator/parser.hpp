#pragma once

#include <vector>
#include "baseClasses.hpp"

class Parser {
public:
    std::vector<Token> parse(std::string const& expr);
private:
    Token parseFunction(std::string const& expr, size_t& pos);
    Token parseNumber(std::string const& expr, size_t& pos);
    Token parseOperatorOrParenthesis(std::string const& expr, size_t& pos);
    std::vector<Token> tokenize(std::string const& expr);
    std::vector<Token> toRPN(std::vector<Token> const& tokens);
};