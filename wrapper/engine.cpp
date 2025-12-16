#include "engine.hpp"
#include "wrapper.hpp"
#include <stdexcept>

void Engine::registerCommand(std::shared_ptr<IWrapper> wrapper, 
                            std::string const& commandName) 
{
    std::lock_guard<std::mutex> lock(mutex);
    commands[commandName] = wrapper;
}

void Engine::registerCommand(IWrapper* wrapper, 
                            std::string const& commandName) 
{
    std::shared_ptr<IWrapper> ptr(wrapper, [](IWrapper*){});
    registerCommand(ptr, commandName);
}

std::any Engine::execute(std::string const& commandName, 
                        std::map<std::string, std::any> const& args) 
{
    std::lock_guard<std::mutex> lock(mutex);
    
    auto it = commands.find(commandName);
    if (it == commands.end()) {
        throw std::invalid_argument("Command not found: " + commandName);
    }
    
    return it->second->execute(args);
}

std::any Engine::execute(std::string const& commandName, 
                        std::initializer_list<std::pair<std::string, std::any>> args) 
{
    return execute(commandName, std::map<std::string, std::any>(args.begin(), args.end()));
}

bool Engine::hasCommand(std::string const& commandName) const 
{
    std::lock_guard<std::mutex> lock(mutex);
    return commands.find(commandName) != commands.end();
}

bool Engine::unregisterCommand(std::string const& commandName) 
{
    std::lock_guard<std::mutex> lock(mutex);
    return commands.erase(commandName) > 0;
}