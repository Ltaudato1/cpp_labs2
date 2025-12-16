#include "wrapper.hpp"
#include <tuple>
#include <vector>
#include <type_traits>
#include <functional>
#include <stdexcept>
#include <map>
#include <any>

template<typename Class, typename ReturnType, typename... Args>
template<typename T>
T Wrapper<Class, ReturnType, Args...>::extractArg(
    std::map<std::string, std::any> const& args, 
    std::string const& name, 
    T const& defaultValue) const 
{
    auto it = args.find(name);
    if (it != args.end()) {
        try {
            return std::any_cast<T>(it->second);
        } catch (std::bad_any_cast const&) {
            throw std::invalid_argument("Argument '" + name + 
                                      "' has wrong type. Expected: " + 
                                      typeid(T).name());
        }
    }
    return defaultValue;
}

template<typename Class, typename ReturnType, typename... Args>
template<size_t... I>
auto Wrapper<Class, ReturnType, Args...>::makeArgsTuple(
    std::map<std::string, std::any> const& args, 
    std::index_sequence<I...>) const 
{
    return std::make_tuple(
        extractArg(args, 
                   argNames[I], 
                   std::get<I>(defaultValues))...
    );
}

template<typename Class, typename ReturnType, typename... Args>
template<size_t... I>
auto Wrapper<Class, ReturnType, Args...>::createDefaultTuple(
    std::vector<std::pair<std::string, std::any>> const& defaultArgs,
    std::index_sequence<I...>) 
{
    return std::make_tuple(
        [&]() -> std::tuple_element_t<I, std::tuple<Args...>> {
            using ExpectedType = std::tuple_element_t<I, std::tuple<Args...>>;
            auto const& [name, value] = defaultArgs[I];
            
            if (value.type() != typeid(ExpectedType)) {
                throw std::invalid_argument(
                    "Default value for argument '" + name + 
                    "' has wrong type. Expected: " + 
                    typeid(ExpectedType).name() + 
                    ", got: " + value.type().name()
                );
            }
            
            return std::any_cast<ExpectedType>(value);
        }()...
    );
}

template<typename Class, typename ReturnType, typename... Args>
Wrapper<Class, ReturnType, Args...>::Wrapper(
    Class* obj, 
    ReturnType (Class::*method)(Args...),
    std::vector<std::pair<std::string, std::any>> const& defaultArgs)
    : obj(obj), 
      method(method),
      defaultValues(createDefaultTuple(defaultArgs, 
                                      std::index_sequence_for<Args...>{})) 
{
    if (defaultArgs.size() != sizeof...(Args)) {
        throw std::invalid_argument("Wrong number of default arguments");
    }
    
    for (auto const& [name, value] : defaultArgs) {
        argNames.push_back(name);
    }
}

template<typename Class, typename ReturnType, typename... Args>
std::any Wrapper<Class, ReturnType, Args...>::execute(
    std::map<std::string, std::any> const& args) 
{
    for (auto const& name : argNames) {
        if (args.find(name) == args.end()) {
            throw std::invalid_argument("Missing required argument: " + name);
        }
    }
    
    auto argsTuple = makeArgsTuple(args, 
                                   std::index_sequence_for<Args...>{});
    
    if constexpr (std::is_same_v<ReturnType, void>) {
        std::apply(method, std::tuple_cat(std::make_tuple(obj), argsTuple));
        return std::any();
    } else {
        return std::apply(method, std::tuple_cat(std::make_tuple(obj), argsTuple));
    }
}