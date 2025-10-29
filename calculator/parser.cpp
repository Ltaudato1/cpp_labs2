#include <iostream>
#include <stack>
#include "baseClasses.hpp"
#include "parser.hpp"

Token Parser::parseNumber(std::string const& expr, size_t& pos) {
    std::string number;
    bool hasDecimalPoint = false;
    
    while (pos < expr.length() && 
           (std::isdigit(expr[pos]) || expr[pos] == '.')) {
        
        if (expr[pos] == '.') {
            if (hasDecimalPoint) {
                throw std::invalid_argument("Invalid number: multiple decimal points");
            }
            hasDecimalPoint = true;
        }
        
        number += expr[pos++];
    }
    
    // Проверяем, что число не заканчивается на точку
    if (number.back() == '.') {
        throw std::invalid_argument("Invalid number: ends with decimal point");
    }
    
    return Token(NUMBER, number);
}

Token Parser::parseOperatorOrParenthesis(std::string const& expr, size_t& pos) {
    char ch = expr[pos++];
    
    switch (ch) {
        case '+': case '-': case '*': case '/': case '^':
            return Token(OPERATION, std::string(1, ch));
        case '(':
            return Token(LEFTPAREN, "(");
        case ')':
            return Token(RIGHTPAREN, ")");
        case ',':
            return Token(COMMA, ",");
        default:
            throw std::invalid_argument("Unknown character: " + std::string(1, ch));
    }
}

Token Parser::parseFunction(std::string const& expr, size_t& pos) {
    std::string name;
    
    while (pos < expr.length() && 
           (std::isalnum(expr[pos]) || expr[pos] == '_')) {
        name += expr[pos++];
    }
    
    return Token(OPERATION, name);
}

std::vector<Token> Parser::tokenize(std::string const& expr) {
    std::vector<Token> tokens;
    size_t pos = 0;
    
    while (pos < expr.length()) {
        // Пропускаем пробелы
        if (std::isspace(expr[pos])) {
            pos++;
            continue;
        }
        
        // Числа (целые, дробные)
        if (std::isdigit(expr[pos]) || expr[pos] == '.') {
            tokens.push_back(parseNumber(expr, pos));
        }
        // Функции
        else if (std::isalpha(expr[pos])) {
            tokens.push_back(parseFunction(expr, pos));
        }
        // Операторы и скобки
        else {
            tokens.push_back(parseOperatorOrParenthesis(expr, pos));
        }
    }
    
    return tokens;
}

std::vector<Token> Parser::toRPN(std::vector<Token> const& tokens) {
    std::vector<Token> output;
    std::stack<Token> stack;
    
    // Таблица приоритетов операций
    auto getPrecedence = [](const std::string& op) {
        if (op == "^") return 4;
        if (op == "*" || op == "/") return 3;
        if (op == "+" || op == "-") return 2;
        return 0; // Для функций и скобок
    };
    
    for (const auto& token : tokens) {
        switch (token.type) {
            case NUMBER:
                output.push_back(token);
                break;
                
            case OPERATION: {
                int currentPrec = getPrecedence(token.value);
                
                while (!stack.empty() && stack.top().type == OPERATION) {
                    int stackPrec = getPrecedence(stack.top().value);
                    if (currentPrec <= stackPrec) {
                        output.push_back(stack.top());
                        stack.pop();
                    } else {
                        break;
                    }
                }
                stack.push(token);
                break;
            }
                
            case LEFTPAREN:
                stack.push(token);
                break;
                
            case RIGHTPAREN:
                while (!stack.empty() && stack.top().type != LEFTPAREN) {
                    output.push_back(stack.top());
                    stack.pop();
                }
                if (!stack.empty()) stack.pop(); // Убираем левую скобку
                break;
                
            default:
                break;
        }
    }
    
    // Выталкиваем оставшиеся операции
    while (!stack.empty()) {
        output.push_back(stack.top());
        stack.pop();
    }
    
    return output;
}

std::vector<Token> Parser::parse(std::string const& expr) {
    auto tokens = tokenize(expr);
    return toRPN(tokens);
}