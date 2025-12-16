#pragma once

#include "iwrapper.hpp"
#include <vector>
#include "wrapper.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>

/**
 * @class Engine
 * @brief Движок для выполнения зарегистрированных команд (обёрнутых методов).
 * 
 * Engine предоставляет централизованный способ регистрации и выполнения команд.
 * Каждая команда представляет собой обёрнутый метод класса, упакованный в IWrapper.
 * 
 * Основные возможности:
 * - Регистрация команд (методов) с назначением им строковых имён
 * - Выполнение команд по имени с переданными аргументами
 * - Проверка наличия команды в реестре
 * - Удаление (отмена регистрации) команд
 * - Потокобезопасность через mutex
 * 
 * @note Engine использует mutex для обеспечения потокобезопасности всех операций.
 * @see IWrapper для интерфейса команд
 * @see Wrapper для конкретной реализации обёрток
 * 
 * Пример использования:
 * @code
 * Subject obj;
 * Wrapper wrapper(&obj, &Subject::method, {{"arg1", 0}, {"arg2", 0}});
 * Engine engine;
 * engine.registerCommand(&wrapper, "myCommand");
 * auto result = engine.execute("myCommand", {{"arg1", 5}, {"arg2", 10}});
 * @endcode
 */
class Engine {
private:
    /// @brief Хранилище зарегистрированных команд: имя -> указатель на обёртку
    std::unordered_map<std::string, std::shared_ptr<IWrapper>> commands;
    
    /// @brief Мьютекс для обеспечения потокобезопасности доступа к хранилищу команд
    mutable std::mutex mutex;
    
public:
    /**
     * @brief Регистрирует команду с переданным именем (версия с shared_ptr).
     * 
     * Команда будет доступна для выполнения под указанным именем.
     * Если команда с таким же именем уже зарегистрирована, она будет перезаписана.
     * 
     * @param wrapper Умный указатель на обёртку метода (shared_ptr)
     * @param commandName Имя команды (уникальный идентификатор)
     * 
     * @note Операция потокобезопасна
     * 
     * @see registerCommand(IWrapper*, const std::string&)
     */
    void registerCommand(std::shared_ptr<IWrapper> wrapper, 
                        std::string const& commandName);
    
    /**
     * @brief Регистрирует команду с переданным именем (версия с обычным указателем).
     * 
     * Удобная перегрузка для регистрации команд со стековым или локальным временем жизни.
     * Внутри преобразует обычный указатель в shared_ptr с пустым deleter'ом,
     * чтобы избежать автоматического удаления объекта.
     * 
     * @param wrapper Указатель на обёртку метода
     * @param commandName Имя команды (уникальный идентификатор)
     * 
     * @warning Убедитесь, что объект обёртки живёт столько же, сколько Engine его использует.
     *          Передача указателя на локальный объект со стека приведёт к undefined behavior.
     * 
     * @note Операция потокобезопасна
     * 
     * @see registerCommand(std::shared_ptr<IWrapper>, const std::string&)
     */
    void registerCommand(IWrapper* wrapper, 
                        std::string const& commandName);
    
    /**
     * @brief Выполняет зарегистрированную команду с переданными аргументами (версия 1).
     * 
     * Ищет команду в реестре и выполняет её, передав переданные аргументы.
     * 
     * @param commandName Имя команды, которую нужно выполнить
     * @param args Map, где ключи — имена параметров, значения — их значения типа std::any
     * 
     * @return std::any Результат выполнения команды, упакованный в std::any.
     *         Для команд, возвращающих void, возвращается пустой std::any.
     * 
     * @throws std::invalid_argument Если команда с таким именем не зарегистрирована
     *         или если переданы неправильные аргументы
     * 
     * @note Операция потокобезопасна
     * 
     * @see execute(const std::string&, std::initializer_list<...>)
     */
    std::any execute(std::string const& commandName, 
                    std::map<std::string, std::any> const& args);
    
    /**
     * @brief Выполняет зарегистрированную команду с переданными аргументами (версия 2).
     * 
     * Удобная перегрузка, позволяющая передавать аргументы через initializer_list.
     * Автоматически преобразует переданный список в std::map и вызывает основную версию execute().
     * 
     * @param commandName Имя команды, которую нужно выполнить
     * @param args Initializer list пар {имя_параметра, значение}
     * 
     * @return std::any Результат выполнения команды, упакованный в std::any.
     *         Для команд, возвращающих void, возвращается пустой std::any.
     * 
     * @throws std::invalid_argument Если команда с таким именем не зарегистрирована
     *         или если переданы неправильные аргументы
     * 
     * @note Операция потокобезопасна
     * 
     * @example
     * @code
     * auto result = engine.execute("sum", {{"arg1", 5}, {"arg2", 10}});
     * @endcode
     * 
     * @see execute(const std::string&, const std::map<std::string, std::any>&)
     */
    std::any execute(std::string const& commandName, 
                    std::initializer_list<std::pair<std::string, std::any>> args);
    
    /**
     * @brief Проверяет, зарегистрирована ли команда с переданным именем.
     * 
     * @param commandName Имя команды для проверки
     * 
     * @return true, если команда с таким именем зарегистрирована; false иначе
     * 
     * @note Операция потокобезопасна (const метод)
     */
    bool hasCommand(std::string const& commandName) const;
    
    /**
     * @brief Удаляет (отменяет регистрацию) команду с переданным именем.
     * 
     * После вызова этого метода команда больше не будет доступна для выполнения.
     * 
     * @param commandName Имя команды, которую нужно удалить
     * 
     * @return true, если команда была успешно удалена; false, если команды с таким именем не было
     * 
     * @note Операция потокобезопасна
     */
    bool unregisterCommand(std::string const& commandName);
};