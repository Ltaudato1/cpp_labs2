#pragma once
#include <vector>
#include <memory>
#include <string>

class IPluginFunction;

class PluginLoader {
public:
    void loadPlugins(std::string const& pluginsDir = "./plugins");
    void unloadAll();

private:
    std::vector<void*> loadedLibraries_;
    
    std::shared_ptr<IPluginFunction> loadPlugin(std::string const& libraryPath);
    void handlePluginError(std::string const& libraryPath, std::string const& error);
};