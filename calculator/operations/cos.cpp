#include "../IPluginFunction.hpp"
#include <stdexcept>
#include <cmath>

extern "C" {
    API const char* getFunctionName() {
        return "cos";
    }
    
    API int getFunctionArity() {
        return 1;
    }
    
    API double executeFunction(const double* args, int argCount) {
        if (argCount != getFunctionArity()) {
            throw std::invalid_argument("Invalid number of arguments");
        }
        return std::cos(args[0]);
    }
    API void registerPluginFunctions() { }
}