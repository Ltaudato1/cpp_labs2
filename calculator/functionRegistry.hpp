#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <filesystem>
#include <functional>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

class FunctionRegistry {
public:
    static FunctionRegistry& getInstance();
    
    void registerFunction(const std::string& name, 
                         std::function<double(const std::vector<double>&)> func,
                         int arity);
    
    bool hasFunction(const std::string& name) const;
    double callFunction(const std::string& name, const std::vector<double>& args) const;
    int getFunctionArity(const std::string& name) const;
    
    void loadPluginsFromDirectory(const std::string& directoryPath="./plugins");
    void loadPlugin(const std::string& pluginPath);
    void unloadAllPlugins();
    
    void clear();

private:
    FunctionRegistry() = default;
    ~FunctionRegistry();
    
    struct FunctionInfo {
        std::function<double(const std::vector<double>&)> function;
        int arity;
    };
    
    std::unordered_map<std::string, FunctionInfo> functions_;
    
    #ifdef _WIN32
        std::vector<HMODULE> loadedLibraries_;
    #else
        std::vector<void*> loadedLibraries_;
    #endif
};