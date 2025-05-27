#ifndef MARKER_DATA_H
#define MARKER_DATA_H

#pragma once
#include <Windows.h>

struct MarkerData {
    int marker_index;
    int size;
    int* arr;
    HANDLE startEvent;
    HANDLE stopEvent;
    HANDLE resumeEvent;
    HANDLE exitEvent;
    CRITICAL_SECTION cs;
};

#endif MARKER_DATA_H