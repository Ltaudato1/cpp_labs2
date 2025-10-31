#pragma once
#include <vector>
#include <string>

#ifdef _WIN32
    #ifdef PLUGIN_EXPORTS
        #define API __declspec(dllexport)
    #else
        #define API __declspec(dllimport)
    #endif
#else
    #define API __attribute__((visibility("default")))
#endif

extern "C" {
    typedef double (*ExecuteFunction)(const double* args, int argCount);
    typedef int (*GetArityFunction)();
    typedef const char* (*GetNameFunction)();
    
    API void registerPluginFunctions();
}