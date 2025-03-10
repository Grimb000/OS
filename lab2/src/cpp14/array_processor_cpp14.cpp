#include "array_processor.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>

DWORD WINAPI MinMaxThread(LPVOID lpParam) {
    ThreadData* data = static_cast<ThreadData*>(lpParam);
    
    std::cout << "MinMax Thread: Searching for min and max values..." << std::endl;
    
    auto minmax_indices = std::make_pair(0, 0);
    auto minmax_values = std::make_pair(data->array[0], data->array[0]);
    
    for (int i = 0; i < data->size; ++i) {
        if (data->array[i] < minmax_values.first) {
            minmax_values.first = data->array[i];
            minmax_indices.first = i;
        }
        Sleep(7);
        
        if (data->array[i] > minmax_values.second) {
            minmax_values.second = data->array[i];
            minmax_indices.second = i;
        }
        Sleep(7);
    }
    
    data->min_value = minmax_values.first;
    data->max_value = minmax_values.second;
    data->min_index = minmax_indices.first;
    data->max_index = minmax_indices.second;
    
    std::cout << "MinMax Thread: Min value = " << data->min_value 
              << " at index " << data->min_index << std::endl;
    std::cout << "MinMax Thread: Max value = " << data->max_value 
              << " at index " << data->max_index << std::endl;
    
    return 0;
}

DWORD WINAPI AverageThread(LPVOID lpParam) {
    ThreadData* data = static_cast<ThreadData*>(lpParam);
    
    int sum = 0;
    std::cout << "Average Thread: Calculating average..." << std::endl;
    
    for (int i = 0; i < data->size; ++i) {
        sum += data->array[i];
        Sleep(12);
    }
    
    data->avg_value = static_cast<double>(sum) / data->size;
    std::cout << "Average Thread: Average value = " << data->avg_value << std::endl;
    
    return 0;
}

void ReadArrayFromConsole(int** array, int* size) {
    std::cout << "Enter array size: ";
    std::cin >> *size;
    
    *array = new int[*size];
    
    std::cout << "Enter " << *size << " integers:\n";
    for (int i = 0; i < *size; ++i) {
        std::cin >> (*array)[i];
    }
}

void ProcessArray(int* array, int size) {
    ThreadData data;
    data.array = array;
    data.size = size;
    
    HANDLE hMinMax = CreateThread(NULL, 0, MinMaxThread, &data, 0, NULL);
    HANDLE hAverage = CreateThread(NULL, 0, AverageThread, &data, 0, NULL);
    
    if (hMinMax == NULL || hAverage == NULL) {
        std::cerr << "Error creating threads\n";
        return;
    }
    
    WaitForSingleObject(hMinMax, INFINITE);
    WaitForSingleObject(hAverage, INFINITE);
    
    CloseHandle(hMinMax);
    CloseHandle(hAverage);
    
    array[data.min_index] = static_cast<int>(data.avg_value);
    array[data.max_index] = static_cast<int>(data.avg_value);
    
    std::cout << "\nMain Thread: Modified array:\n";
    for (int i = 0; i < size; ++i) {
        std::cout << array[i] << " ";
    }
    std::cout << std::endl;
}
