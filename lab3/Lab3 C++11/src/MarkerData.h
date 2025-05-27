#ifndef MARKER_DATA_H
#define MARKER_DATA_H

#pragma once
#include <Windows.h>
#include <mutex>

struct MarkerData {

    int marker_index;
    int size;
    int* arr;
    HANDLE startEvent;
    HANDLE stopEvent;
    HANDLE resumeEvent;
    HANDLE exitEvent;
    std::mutex* mtx;

};

#endif MARKER_DATA_H