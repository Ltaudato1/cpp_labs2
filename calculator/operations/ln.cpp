#include "../IPluginFunction.hpp"
#include <cmath>
#include <stdexcept>

extern "C" {
    API const char* getFunctionName() {
        return "ln";
    }
    
    API int getFunctionArity() {
        return 1;
    }
    
    API double executeFunction(const double* args, int argCount) {
        if (argCount != getFunctionArity()) {
            throw std::invalid_argument("Invalid number of arguments");
        }

        if (args[0] < 0.0) {
            throw std::invalid_argument("Logarithm of non-positive number");
        }

        if (args[0] == 0.0) {
            throw std::invalid_argument("Logarithm of zero");
        }

        return std::log(args[0]);
    }
    API void registerPluginFunctions() { }
}