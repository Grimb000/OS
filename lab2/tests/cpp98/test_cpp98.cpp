#include "array_processor.h"
#include <iostream>
#include <cassert>

// Тестовый поток для минимального и максимального значений
DWORD WINAPI TestMinMaxThread(LPVOID lpParam) {
    ThreadData* data = static_cast<ThreadData*>(lpParam);
    MinMaxThread(data);
    return 0;
}

// Тестовый поток для среднего значения
DWORD WINAPI TestAverageThread(LPVOID lpParam) {
    ThreadData* data = static_cast<ThreadData*>(lpParam);
    AverageThread(data);
    return 0;
}

// Тестирование функций
void RunTests() {
    std::cout << "Running C++98 tests...\n";
    
    // Тест 1: Простой массив
    {
        int test_array[] = {5, 2, 8, 1, 9};
        int size = sizeof(test_array) / sizeof(test_array[0]);
        
        ThreadData data;
        data.array = test_array;
        data.size = size;
        
        HANDLE hMinMax = CreateThread(NULL, 0, TestMinMaxThread, &data, 0, NULL);
        WaitForSingleObject(hMinMax, INFINITE);
        CloseHandle(hMinMax);
        
        assert(data.min_value == 1);
        assert(data.max_value == 9);
        assert(data.min_index == 3);
        assert(data.max_index == 4);
        
        std::cout << "Test 1 (MinMax) passed!\n";
        
        HANDLE hAverage = CreateThread(NULL, 0, TestAverageThread, &data, 0, NULL);
        WaitForSingleObject(hAverage, INFINITE);
        CloseHandle(hAverage);
        
        assert(data.avg_value == 5.0);
        
        std::cout << "Test 1 (Average) passed!\n";
    }
    
    // Тест 2: Массив с отрицательными числами
    {
        int test_array[] = {-5, 2, -8, 1, -9};
        int size = sizeof(test_array) / sizeof(test_array[0]);
        
        ThreadData data;
        data.array = test_array;
        data.size = size;
        
        HANDLE hMinMax = CreateThread(NULL, 0, TestMinMaxThread, &data, 0, NULL);
        WaitForSingleObject(hMinMax, INFINITE);
        CloseHandle(hMinMax);
        
        assert(data.min_value == -9);
        assert(data.max_value == 2);
        assert(data.min_index == 4);
        assert(data.max_index == 1);
        
        std::cout << "Test 2 (MinMax) passed!\n";
        
        HANDLE hAverage = CreateThread(NULL, 0, TestAverageThread, &data, 0, NULL);
        WaitForSingleObject(hAverage, INFINITE);
        CloseHandle(hAverage);
        
        assert(data.avg_value == -3.8);
        
        std::cout << "Test 2 (Average) passed!\n";
    }
    
    // Тест 3: Массив с одинаковыми элементами
    {
        int test_array[] = {7, 7, 7, 7, 7};
        int size = sizeof(test_array) / sizeof(test_array[0]);
        
        ThreadData data;
        data.array = test_array;
        data.size = size;
        
        HANDLE hMinMax = CreateThread(NULL, 0, TestMinMaxThread, &data, 0, NULL);
        WaitForSingleObject(hMinMax, INFINITE);
        CloseHandle(hMinMax);
        
        assert(data.min_value == 7);
        assert(data.max_value == 7);
        assert(data.min_index == 0);
        assert(data.max_index == 0);
        
        std::cout << "Test 3 (MinMax) passed!\n";
        
        HANDLE hAverage = CreateThread(NULL, 0, TestAverageThread, &data, 0, NULL);
        WaitForSingleObject(hAverage, INFINITE);
        CloseHandle(hAverage);
        
        assert(data.avg_value == 7.0);
        
        std::cout << "Test 3 (Average) passed!\n";
    }
    
    std::cout << "All C++98 tests passed!\n";
}

int main() {
    RunTests();
    return 0;
}
