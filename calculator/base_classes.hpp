#pragma once

#include <string>

enum Type { 
    NUMBER, 
    OPERATOR, 
    FUNCTION, 
    LEFTPAREN, 
    RIGHTPATEN 
};

class Token{
    public:
        Type type;
        std::string value;
};