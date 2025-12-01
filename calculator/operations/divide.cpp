#include "../IPluginFunction.hpp"
#include <stdexcept>
#include <cmath>

extern "C" {
    API const char* getFunctionName() {
        return "/";
    }
    
    API int getFunctionArity() {
        return 2;
    }
    
    API double executeFunction(const double* args, int argCount) {
        if (argCount != getFunctionArity()) {
            throw std::invalid_argument("Invalid number of arguments");
        }
        
        if (args[1] == 0.0) {
            throw std::invalid_argument("Division by zero");
        }
        
        return args[0] / args[1];
    }
    API void registerPluginFunctions() { }
}