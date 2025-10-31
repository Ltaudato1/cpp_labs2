#include "../IPluginFunction.hpp"
#include <cmath>

extern "C" {
    API const char* getFunctionName() {
        return "^";
    }
    
    API int getFunctionArity() {
        return 2;
    }
    
    API double executeFunction(const double* args, int argCount) {
        if (argCount != getFunctionArity()) {
            return 0.0;
        }
        return std::pow(args[0], args[1]);
    }

    API void registerPluginFunctions() { }
}