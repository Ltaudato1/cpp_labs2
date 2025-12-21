# Wrapper Engine

## Структура проекта

```
wrapper/
├── engine.hpp           # Интерфейс движка команд
├── engine.cpp           # Реализация Engine
├── iwrapper.hpp         # Интерфейс обёртки
├── wrapper.hpp          # Шаблонная реализация обёртки
├── wrapper.tpp          # Реализация шаблонных методов
├── main.cpp             # Точка входа
├── tests.hpp            # Заголовок тестов
├── tests.cpp            # Реализация тестов
└── CMakeLists.txt       # Конфигурация сборки
```

## Требования

- **C++17** или выше
- **CMake 3.10+**
- **Компилятор**: GCC 7+, Clang 5+, или MSVC 2017+

## Сборка проекта

### Linux/macOS

```bash
cd /path/to/cpp_labs2/wrapper

# Создание директории сборки
mkdir -p build
cd build

# Конфигурация и сборка
cmake ..
make

# Запуск программы с тестами
./wrapper_engine
```

### Windows (Visual Studio)

```bash
cd path\to\cpp_labs2\wrapper

# Создание директории сборки
mkdir build
cd build

# Генерация Visual Studio проекта
cmake .. -G "Visual Studio 17 2022"

# Сборка
cmake --build . --config Release

# Запуск программы
Release\wrapper_engine.exe
```

### Windows (MinGW)

```bash
cd path\to\cpp_labs2\wrapper
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make
wrapper_engine.exe
```

### Debug сборка

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
./wrapper_engine
```

### Release сборка

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./wrapper_engine
```

## Использование

### Базовый пример

```cpp
#include "engine.hpp"
#include "wrapper.hpp"

class Calculator {
public:
    int add(int a, int b) { return a + b; }
};

int main() {
    Calculator calc;
    
    // Создание обёртки
    Wrapper wrapper(&calc, &Calculator::add, {
        {"a", 0}, 
        {"b", 0}
    });
    
    // Создание движка и регистрация команды
    Engine engine;
    engine.registerCommand(&wrapper, "add");
    
    // Выполнение команды
    auto result = engine.execute("add", {
        {"a", 5}, 
        {"b", 3}
    });
    
    // Получение результата
    int sum = std::any_cast<int>(result);  // sum == 8
    
    return 0;
}
```

## Архитектура

### IWrapper

Интерфейс для обёрток методов класса. Определяет контракт для выполнения метода с произвольными параметрами через `std::any`.

### Wrapper<Class, ReturnType, Args...>

Шаблонная реализация обёртки для конкретного метода. Осуществляет:
- Упаковку/распаковку параметров из `std::any`
- Проверку типов параметров
- Вызов обёрнутого метода
- Упаковку результата в `std::any`

### Engine

Централизованный реестр команд. Обеспечивает:
- Регистрацию команд по строковым именам
- Выполнение команд
- Проверку наличия команды
- Удаление команд
- Потокобезопасность всех операций

## Тестирование

Проект включает полный набор юнит-тестов, проверяющих:

1. **Basic functionality** — простой вызов метода с возвращаемым значением
2. **Different types** — методы с разными типами параметров (int, double, std::string)
3. **Void function** — методы, не возвращающие значение
4. **Error handling** — обработка ошибок (неизвестная команда, неправильный тип)
5. **Multithreading safety** — потокобезопасность операций

Запуск тестов:

```bash
./wrapper_engine
```

Ожидаемый результат:

```
Test 1: Basic functionality
Result: 9 (expected: 9)

Test 2: Different types
Multiplication result: 7.5 (expected: 7.5)
Concatenation result: Hello, World!

Test 3: Void function
Sum: 30
Void function executed successfully

Test 4: Error handling
Expected error (unknown command): Command not found: unknown_command
Expected error (wrong type): Argument 'arg2' has wrong type...
Expected error (missing argument): Missing required argument: arg2

Test 5: Multithreading safety
Multithreading operations completed successfully

All tests passed successfully!
```

## API Документация

Полная документация кода доступна через Doxygen комментарии в заголовочных файлах:

- `engine.hpp` — документация Engine и его методов
- `iwrapper.hpp` — документация интерфейса IWrapper
- `wrapper.hpp` — документация шаблонного класса Wrapper

## Примечания

- Объект обёрнутого класса должен существовать столько же времени, сколько существует Wrapper
- Порядок параметров в `defaultArgs` должен совпадать с порядком в сигнатуре метода
- Engine потокобезопасен благодаря использованию `std::mutex`
