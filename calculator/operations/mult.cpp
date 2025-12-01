#include "../IPluginFunction.hpp"
#include <stdexcept>

extern "C" {
    API const char* getFunctionName() {
        return "*";
    }
    
    API int getFunctionArity() {
        return 2;
    }
    
    API double executeFunction(const double* args, int argCount) {
        if (argCount != getFunctionArity()) {
            throw std::invalid_argument("Invalid number of arguments");
        }
        return args[0] * args[1];
    }
    API void registerPluginFunctions() { }
}