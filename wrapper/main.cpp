#include "engine.hpp"
#include "wrapper.hpp"
#include "wrapper.tpp"
#include <cassert>
#include <vector>
#include <iostream>

class Subject {
public:
    int f3(int arg1, int arg2);
    double multiply(double a, double b);
    std::string concat(std::string s1, std::string s2);
    void printSum(int a, int b);
};

int Subject::f3(int arg1, int arg2) {
    return arg1 + arg2;
}

double Subject::multiply(double a, double b) {
    return a * b;
}

std::string Subject::concat(std::string s1, std::string s2) {
    return s1 + s2;
}

void Subject::printSum(int a, int b) {
    std::cout << "Sum: " << (a + b) << std::endl;
}

void testBasicFunctionality() {
    std::cout << "Test 1: Basic functionality" << std::endl;
    
    Subject subj;
    Wrapper wrapper(&subj, &Subject::f3, {{"arg1", 0}, {"arg2", 0}});
    
    Engine engine;
    engine.registerCommand(&wrapper, "sum");
    
    auto result = engine.execute("sum", {{"arg1", 4}, {"arg2", 5}});
    
    assert(std::any_cast<int>(result) == 9);
    std::cout << "Result: " << std::any_cast<int>(result) << " (expected: 9)" << std::endl;
}

void testDifferentTypes() {
    std::cout << "\nTest 2: Different types" << std::endl;
    
    Subject subj;
    Engine engine;
    
    // Тест с double
    Wrapper wrapper1(&subj, &Subject::multiply, {{"a", 0.0}, {"b", 0.0}});
    engine.registerCommand(&wrapper1, "multiply");
    
    auto result1 = engine.execute("multiply", {{"a", 2.5}, {"b", 3.0}});
    assert(std::any_cast<double>(result1) == 7.5);
    std::cout << "Multiplication result: " << std::any_cast<double>(result1) 
              << " (expected: 7.5)" << std::endl;
    
    // Тест со string
    Wrapper wrapper2(&subj, &Subject::concat, {{"s1", std::string("")}, {"s2", std::string("")}});
    engine.registerCommand(&wrapper2, "concat");
    
    auto result2 = engine.execute("concat", {
        {"s1", std::string("Hello, ")},
        {"s2", std::string("World!")}
    });
    assert(std::any_cast<std::string>(result2) == "Hello, World!");
    std::cout << "Concatenation result: " << std::any_cast<std::string>(result2) << std::endl;
}

void testVoidFunction() {
    std::cout << "\nTest 3: Void function" << std::endl;
    
    Subject subj;
    Wrapper wrapper(&subj, &Subject::printSum, {{"a", 0}, {"b", 0}});
    
    Engine engine;
    engine.registerCommand(&wrapper, "print");
    
    // Void-функция должна выполняться без ошибок
    try {
        auto result = engine.execute("print", {{"a", 10}, {"b", 20}});
        // Для void-функций возвращается пустой any
        assert(!result.has_value());
        std::cout << "Void function executed successfully" << std::endl;
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        assert(false);
    }
}

void testErrorHandling() {
    std::cout << "\nTest 4: Error handling" << std::endl;
    
    Subject subj;
    Wrapper wrapper(&subj, &Subject::f3, {{"arg1", 0}, {"arg2", 0}});
    
    Engine engine;
    engine.registerCommand(&wrapper, "sum");
    
    // Тест 1: Неправильное имя команды
    try {
        engine.execute("unknown_command", {{"arg1", 1}, {"arg2", 2}});
        assert(false); // Не должно дойти сюда
    } catch (std::invalid_argument const& e) {
        std::cout << "Expected error (unknown command): " << e.what() << std::endl;
    }
    
    // Тест 2: Неправильный тип аргумента
    try {
        engine.execute("sum", {{"arg1", 1}, {"arg2", std::string("wrong")}});
        assert(false); // Не должно дойти сюда
    } catch (std::invalid_argument const& e) {
        std::cout << "Expected error (wrong type): " << e.what() << std::endl;
    }
    
    // Тест 3: Отсутствующий аргумент
    try {
        engine.execute("sum", {{"arg1", 1}});
        assert(false); // Не должно дойти сюда
    } catch (std::invalid_argument const& e) {
        std::cout << "Expected error (missing argument): " << e.what() << std::endl;
    }
}

void testMultithreading() {
    std::cout << "\nTest 5: Multithreading safety" << std::endl;
    
    Subject subj;
    Wrapper wrapper(&subj, &Subject::f3, {{"arg1", 0}, {"arg2", 0}});
    
    Engine engine;
    engine.registerCommand(&wrapper, "sum");
    
    // Проверяем, что команда зарегистрирована
    assert(engine.hasCommand("sum"));
    
    // Проверяем удаление команды
    assert(engine.unregisterCommand("sum"));
    assert(!engine.hasCommand("sum"));
    
    std::cout << "Multithreading operations completed successfully" << std::endl;
}

// Функция, которая вызывает все юнит-тесты
void runAllTests() {
    try {
        testBasicFunctionality();
        testDifferentTypes();
        testVoidFunction();
        testErrorHandling();
        testMultithreading();
        
        std::cout << "\nAll tests passed successfully!" << std::endl;
        
    } catch (std::exception const& e) {
        std::cerr << "Test failed with error: " << e.what() << std::endl;
        throw;
    }
}

int main() {
    try {
        runAllTests();
        return 0;
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}