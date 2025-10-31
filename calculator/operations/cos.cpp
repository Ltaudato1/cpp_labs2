#include "../IPluginFunction.hpp"
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
            return 0.0;
        }
        return std::cos(args[0]);
    }
    API void registerPluginFunctions() { }
}