#pragma once

#include <string>

enum Type { 
    NUMBER, 
    OPERATION,
    FUNCTION,
    LEFTPAREN, 
    RIGHTPAREN,
    COMMA 
};

class Token{
    public:
        Type type;
        std::string value;
        Token(Type t, std::string const& val): type(t), value(val) {}
};
