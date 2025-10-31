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
    typedef double (*ExecuteFunction)(double const* args, int argCount);
    typedef int (*GetArityFunction)();
    typedef char const* (*GetNameFunction)();
    
    API void registerPluginFunctions();
}