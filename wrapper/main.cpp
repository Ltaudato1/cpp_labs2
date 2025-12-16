#include "tests.hpp"
#include <iostream>

int main() {
    try {
        runAllTests();
        return 0;
    } catch (std::exception const& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}