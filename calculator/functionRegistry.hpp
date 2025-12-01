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
    
    void registerFunction(std::string const& name, 
                         std::function<double(std::vector<double> const&)> func,
                         int arity);
    
    bool hasFunction(std::string const& name) const;
    double callFunction(std::string const& name, const std::vector<double>& args) const;
    int getFunctionArity(std::string const& name) const;
    
    void loadPluginsFromDirectory(
            std::string const& directoryPath="plugins"
    );
    void loadPlugin(std::string const& pluginPath);
    void unloadAllPlugins();
    
    void clear();

private:
    FunctionRegistry() = default;
    ~FunctionRegistry();
    
    struct FunctionInfo {
        std::function<double(std::vector<double> const&)> function;
        int arity;
    };
    
    std::unordered_map<std::string, FunctionInfo> functions_;
    
    #ifdef _WIN32
        std::vector<HMODULE> loadedLibraries_;
    #else
        std::vector<void*> loadedLibraries_;
    #endif
};