#pragma once
#include <vector>
#include <memory>
#include <string>

class IPluginFunction;

class PluginLoader {
public:
    void loadPlugins(const std::string& pluginsDir = "./plugins");
    void unloadAll();

private:
    std::vector<void*> loadedLibraries_;
    
    std::shared_ptr<IPluginFunction> loadPlugin(const std::string& libraryPath);
    void handlePluginError(const std::string& libraryPath, const std::string& error);
};