#pragma once

#include "iwrapper.hpp"
#include <tuple>
#include <map>
#include <vector>
#include <any>
#include <type_traits>
#include <functional>
#include <stdexcept>

/**
 * @class Wrapper
 * @brief Шаблонная обёртка для методов класса с произвольным числом параметров.
 * 
 * Wrapper позволяет упаковать метод класса и его параметры в единый интерфейс IWrapper,
 * скрывая типовую информацию. Это особенно полезно при работе с методами, типы которых
 * неизвестны на этапе компиляции, или при необходимости вызывать различные методы
 * через единый интерфейс.
 * 
 * @tparam Class Класс, методы которого обёртываются
 * @tparam ReturnType Тип возвращаемого значения метода (может быть void)
 * @tparam Args Типы параметров метода (может быть пусто)
 * 
 * Основные возможности:
 * - Упаковка методов с любым числом параметров
 * - Поддержка методов, возвращающих void
 * - Значения по умолчанию для параметров
 * - Автоматическое преобразование типов из/в std::any
 * - Проверка типов параметров
 * 
 * @note Объект обёртываемого класса должен существовать столько же времени, сколько существует Wrapper.
 * 
 * @see IWrapper для интерфейса
 * @see Engine для использования с движком команд
 * 
 * Пример использования:
 * @code
 * class Calculator {
 * public:
 *     int add(int a, int b) { return a + b; }
 * };
 * 
 * Calculator calc;
 * Wrapper<Calculator, int, int, int> wrapper(
 *     &calc, 
 *     &Calculator::add, 
 *     {{"a", 0}, {"b", 0}}
 * );
 * 
 * auto result = wrapper.execute({{"a", 5}, {"b", 3}});
 * int sum = std::any_cast<int>(result); // sum == 8
 * @endcode
 */
template<typename Class, typename ReturnType, typename... Args>
class Wrapper : public IWrapper {
private:
    /// @brief Указатель на объект класса, чьи методы обёртываются
    Class* obj;
    
    /// @brief Указатель на метод класса
    ReturnType (Class::*method)(Args...);
    
    /// @brief Вектор имён параметров в порядке их появления в сигнатуре метода
    std::vector<std::string> argNames;
    
    /// @brief Кортеж значений по умолчанию для параметров
    std::tuple<Args...> defaultValues;

    /**
     * @brief Извлекает аргумент из map по имени с проверкой типа.
     * 
     * Функция ищет параметр с переданным именем в map. Если его там нет,
     * используется значение по умолчанию. Если он есть, проверяется его тип
     * и выполняется преобразование из std::any.
     * 
     * @tparam T Тип извлекаемого параметра
     * @param args Map аргументов типа name -> value
     * @param name Имя параметра для поиска
     * @param defaultValue Значение по умолчанию, если параметр не найден
     * 
     * @return Значение параметра типа T
     * 
     * @throws std::invalid_argument Если параметр присутствует в map, но имеет неправильный тип
     * 
     * @note Метод работает с любыми типами, поддерживаемыми std::any_cast
     */
    template<typename T>
    T extractArg(std::map<std::string, std::any> const& args, 
                 std::string const& name, 
                 T const& defaultValue) const;

    /**
     * @brief Создаёт кортеж аргументов из map с использованием значений по умолчанию.
     * 
     * Внутренний вспомогательный метод, использующий parameter pack для распаковки
     * последовательности индексов и создания кортежа из переданных аргументов.
     * 
     * @tparam I Последовательность индексов (обычно 0, 1, 2, ... sizeof...(Args)-1)
     * @param args Map с аргументами для метода
     * @param Используется для SFINAE распаковки параметров
     * 
     * @return std::tuple<Args...> Кортеж готовых к использованию аргументов
     * 
     * @note Это внутренний метод, используется только из execute()
     */
    template<size_t... I>
    auto makeArgsTuple(std::map<std::string, std::any> const& args, 
                       std::index_sequence<I...>) const;

    /**
     * @brief Создаёт кортеж значений по умолчанию из initializer_list.
     * 
     * Внутренний вспомогательный метод для инициализации defaultValues из переданного
     * initializer_list пар {имя, значение}.
     * 
     * @tparam I Последовательность индексов
     * @param defaultArgs Вектор пар {имя_параметра, значение_по_умолчанию}
     * @param Используется для SFINAE распаковки параметров
     * 
     * @return std::tuple<Args...> Кортеж значений по умолчанию
     * 
     * @note Это внутренний метод, используется только в конструкторе
     */
    template<size_t... I>
    auto createDefaultTuple(std::vector<std::pair<std::string, std::any>> const& defaultArgs,
                            std::index_sequence<I...>);
    
public:
    /**
     * @brief Конструктор обёртки для метода класса.
     * 
     * Создаёт обёртку для метода класса, сохраняя указатель на объект, указатель на метод,
     * имена параметров и их значения по умолчанию.
     * 
     * @param obj Указатель на объект класса, метод которого обёртывается
     * @param method Указатель на метод класса (синтаксис &Class::methodName)
     * @param defaultArgs Вектор пар {имя_параметра, значение_по_умолчанию}
     *        в порядке появления параметров в сигнатуре метода
     * 
     * @warning Убедитесь, что obj живёт столько же, сколько существует Wrapper.
     *          Передача указателя на локальный объект приведёт к undefined behavior.
     * 
     * @warning Порядок параметров в defaultArgs должен совпадать с порядком параметров в методе.
     *          Неправильный порядок приведёт к неправильным результатам.
     * 
     * @example
     * @code
     * class Calc { 
     * public: 
     *     int sum(int a, int b); 
     * };
     * 
     * Calc calc;
     * Wrapper wrapper(&calc, &Calc::sum, {{"a", 0}, {"b", 0}});
     * @endcode
     */
    Wrapper(Class* obj, 
            ReturnType (Class::*method)(Args...),
            std::vector<std::pair<std::string, std::any>> const& defaultArgs);
    
    /**
     * @brief Выполняет обёрнутый метод с переданными аргументами.
     * 
     * Основной метод интерфейса IWrapper. Преобразует аргументы из std::any в правильные типы,
     * вызывает метод, и упаковывает результат обратно в std::any.
     * 
     * @param args Map, где ключи — имена параметров, значения — их значения типа std::any.
     *        Если параметр не указан, используется значение по умолчанию.
     * 
     * @return std::any Результат выполнения метода, упакованный в std::any.
     *         Для методов, возвращающих void, возвращается пустой std::any.
     * 
     * @throws std::invalid_argument Если:
     *         - Параметр присутствует в args, но имеет неправильный тип
     *         - Не удалось выполнить преобразование std::any_cast
     * 
     * @note Метод переопределяет чистый виртуальный метод из IWrapper
     * 
     * @example
     * @code
     * auto result = wrapper.execute({{"a", 10}, {"b", 20}});
     * int value = std::any_cast<int>(result); // value == 30 для sum
     * @endcode
     */
    std::any execute(std::map<std::string, std::any> const& args) override;
};