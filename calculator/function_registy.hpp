#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class FunctionRegistry {
public:
    using UnaryFunc = double(*)(double);
    using BinaryFunc = double(*)(double, double);

    void Register(std::string const& name, UnaryFunc func);
    void Register(std::string const& name, BinaryFunc func);
    bool Has(std::string const& name);
    double Call(std::string const& name, std::vector<double> args);

private:
    std::unordered_map<std::string, UnaryFunc> unary;
    std::unordered_map<std::string, BinaryFunc> binary;
};
