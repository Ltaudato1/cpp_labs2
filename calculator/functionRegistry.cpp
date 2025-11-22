#include "functionRegistry.hpp"
#include "IPluginFunction.hpp"
#include <iostream>

void FunctionRegistry::registerFunction(std::string const& name, 
                                       std::function<double(std::vector<double> const&)> func,
                                       int arity) {
    functions_[name] = {func, arity};
}

bool FunctionRegistry::hasFunction(std::string const& name) const {
    if (functions_.find(name) == functions_.end()) {
        FunctionRegistry::getInstance().loadPluginsFromDirectory();
        return functions_.find(name) != functions_.end();
    };
    return true;
}

double FunctionRegistry::callFunction(std::string const& name, std::vector<double> const& args) const {
    if (!hasFunction(name)) {
        throw std::runtime_error("Function not found: " + name);
    }
    auto const& funcInfo = functions_.find(name)->second;
    if (funcInfo.arity != -1 && args.size() != static_cast<size_t>(funcInfo.arity)) {
        throw std::runtime_error("Function " + name + " expects " + 
                               std::to_string(funcInfo.arity) + " arguments");
    }

    try {
        return funcInfo.function(args);
    } catch(std::exception const& e) {
        throw std::runtime_error("Error in function '" + name + "': " + e.what());
    }
}

int FunctionRegistry::getFunctionArity(std::string const& name) const {
    auto it = functions_.find(name);
    return it != functions_.end() ? it->second.arity : -1;
}

void FunctionRegistry::loadPluginsFromDirectory(std::string const& directoryPath) {
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
    } catch (fs::filesystem_error const& ex) {
        std::cerr << "Error accessing directory: " << ex.what() << std::endl;
    }
}

void FunctionRegistry::loadPlugin(std::string const& pluginPath) {
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
            auto executeFunc = (ExecuteFunction)GetProcAddress(library, "executeFunction");
        #else
            auto getNameFunc = (GetNameFunction)dlsym(library, "getFunctionName");
            auto getArityFunc = (GetArityFunction)dlsym(library, "getFunctionArity");
            auto executeFunc = (ExecuteFunction)dlsym(library, "executeFunction");
        #endif
        
        if (getNameFunc && executeFunc) {
            const char* functionName = getNameFunc();
            int arity = getArityFunc ? getArityFunc() : -1; // -1 для переменного числа аргументов
            
            // Обертка для преобразования формата вызова
            auto wrappedFunc = [executeFunc](std::vector<double> const& args) -> double {
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