#include "array_processor.h"
#include <iostream>

int main() {
    int* array = nullptr;
    int size = 0;
    
    std::cout << "Array Processor (C++14 version)\n";
    
    ReadArrayFromConsole(&array, &size);
    
    ProcessArray(array, size);
    
    delete[] array;
    
    return 0;
}
