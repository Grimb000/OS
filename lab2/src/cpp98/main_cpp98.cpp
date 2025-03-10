#include "array_processor.h"
#include <iostream>

int main() {
    int* array = NULL;
    int size = 0;
    
    std::cout << "Array Processor (C++98 version)\n";
    
    ReadArrayFromConsole(&array, &size);
    
    ProcessArray(array, size);
    
    delete[] array;
    
    return 0;
}
