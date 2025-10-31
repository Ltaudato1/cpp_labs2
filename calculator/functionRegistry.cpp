#include "functionRegistry.hpp"
#include "IPluginFunction.hpp"
#include <iostream>

void FunctionRegistry::registerFunction(const std::string& name, 
                                       std::function<double(const std::vector<double>&)> func,
                                       int arity) {
    functions_[name] = {func, arity};
}

bool FunctionRegistry::hasFunction(const std::string& name) const {
    if (functions_.find(name) == functions_.end()) {
        FunctionRegistry::getInstance().loadPluginsFromDirectory();
        return functions_.find(name) != functions_.end();
    };
    return true;
}

double FunctionRegistry::callFunction(const std::string& name, const std::vector<double>& args) const {
    if (!hasFunction(name)) {
        throw std::runtime_error("Function not found: " + name);
    }
    const auto& funcInfo = functions_.find(name)->second;
    if (funcInfo.arity != -1 && args.size() != static_cast<size_t>(funcInfo.arity)) {
        throw std::runtime_error("Function " + name + " expects " + 
                               std::to_string(funcInfo.arity) + " arguments");
    }
    
    try {
        double res = funcInfo.function(args);
        return res;
    } catch(std::exception e) {
        return NULL;
    }
}

int FunctionRegistry::getFunctionArity(const std::string& name) const {
    auto it = functions_.find(name);
    return it != functions_.end() ? it->second.arity : -1;
}

void FunctionRegistry::loadPluginsFromDirectory(const std::string& directoryPath) {
    namespace fs = std::filesystem;
    
    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                
                if (extension == ".dll") {
                    loadPlugin(entry.path().string());
                }
            }
        }
    } catch (const fs::filesystem_error& ex) {
        std::cerr << "Error accessing directory: " << ex.what() << std::endl;
    }
}

void FunctionRegistry::loadPlugin(const std::string& pluginPath) {
    #ifdef _WIN32
        HMODULE library = LoadLibraryA(pluginPath.c_str());
    #else
        void* library = dlopen(pluginPath.c_str(), RTLD_LAZY);
    #endif
    
    if (!library) {
        std::cerr << "Failed to load library: " << pluginPath << std::endl;
        return;
    }
    
    // Загружаем функцию регистрации
    #ifdef _WIN32
        auto registerFunc = (void(*)())GetProcAddress(library, "registerPluginFunctions");
    #else
        auto registerFunc = (void(*)())dlsym(library, "registerPluginFunctions");
    #endif
    
    if (registerFunc) {
        // Получаем все необходимые функции из DLL
        #ifdef _WIN32
            auto getNameFunc = (GetNameFunction)GetProcAddress(library, "getFunctionName");
            auto getArityFunc = (GetArityFunction)GetProcAddress(library, "getFunctionArity");
            auto executeFunc = (PluginFunction)GetProcAddress(library, "executeFunction");
        #else
            auto getNameFunc = (GetNameFunction)dlsym(library, "getFunctionName");
            auto getArityFunc = (GetArityFunction)dlsym(library, "getFunctionArity");
            auto executeFunc = (ExecuteFunction)dlsym(library, "executeFunction");
        #endif
        
        if (getNameFunc && executeFunc) {
            const char* functionName = getNameFunc();
            int arity = getArityFunc ? getArityFunc() : -1; // -1 для переменного числа аргументов
            
            // Обертка для преобразования формата вызова
            auto wrappedFunc = [executeFunc](const std::vector<double>& args) -> double {
                return executeFunc(args.data(), static_cast<int>(args.size()));
            };
            
            registerFunction(functionName, wrappedFunc, arity);
            loadedLibraries_.push_back(library);
            std::cout << "Registered function: " << functionName << " from " << pluginPath << std::endl;
        } else {
            std::cerr << "Missing required functions in: " << pluginPath << std::endl;
            #ifdef _WIN32
                FreeLibrary(library);
            #else
                dlclose(library);
            #endif
        }
    } else {
        std::cerr << "No registerPluginFunctions found in: " << pluginPath << std::endl;
        #ifdef _WIN32
            FreeLibrary(library);
        #else
            dlclose(library);
        #endif
    }
}

FunctionRegistry::~FunctionRegistry() {
    unloadAllPlugins();
}

void FunctionRegistry::unloadAllPlugins() {
    for (auto library : loadedLibraries_) {
        #ifdef _WIN32
            FreeLibrary(library);
        #else
            dlclose(library);
        #endif
    }
    loadedLibraries_.clear();
    functions_.clear();
}

void FunctionRegistry::clear() {
    functions_.clear();
}

FunctionRegistry& FunctionRegistry::getInstance() {
    static FunctionRegistry instance;
    return instance;
}