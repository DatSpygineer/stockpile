#include "stockpile.h"

#include <iostream>

int main() {
    StpSetErrorCallback([](StpErrorCode code, const char* msg) { std::cout << "Error " << code << ": " << msg << std::endl; exit(code); });

    

    return 0;
}