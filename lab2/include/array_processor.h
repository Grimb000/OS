#ifndef ARRAY_PROCESSOR_H
#define ARRAY_PROCESSOR_H

#include <windows.h>

struct ThreadData {
    int* array;
    int size;
    int min_value;
    int max_value;
    double avg_value;
    int min_index;
    int max_index;
};

DWORD WINAPI MinMaxThread(LPVOID lpParam);
DWORD WINAPI AverageThread(LPVOID lpParam);

void ReadArrayFromConsole(int** array, int* size);

void ProcessArray(int* array, int size);

#endif
