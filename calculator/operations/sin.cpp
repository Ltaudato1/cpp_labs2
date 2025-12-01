#include "../IPluginFunction.hpp"
#include <cmath>
#include <stdexcept>

extern "C" {
    API const char* getFunctionName() {
        return "sin";
    }
    
    API int getFunctionArity() {
        return 1;
    }
    
    API double executeFunction(const double* args, int argCount) {
        if (argCount != getFunctionArity()) {
            throw std::invalid_argument("Invalid number of arguments");
        }
        return std::sin(args[0]);
    }
    API void registerPluginFunctions() { }
}