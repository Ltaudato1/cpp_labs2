#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "IPluginFunction.hpp"

class FunctionRegistry {
public:
    static FunctionRegistry& getInstance();
    
    void registerFunction(const std::string& name, std::shared_ptr<IPluginFunction> func);
    bool hasFunction(const std::string& name) const;
    double callFunction(const std::string& name, const std::vector<double>& args) const;
    size_t getFunctionArity(const std::string& name) const;
    
    void clear();

private:
    FunctionRegistry() = default;
    std::unordered_map<std::string, std::shared_ptr<IPluginFunction>> functions_;
};