#include "marker.h"
#include <iostream>


int main() {
    int size, numThreads;
    std::cout << "Enter array size: ";
    std::cin >> size;

    int* array = new int[size];
    for (int i = 0; i < size; ++i) {
        array[i] = 0;
    }

    std::cout << "Enter number of marker threads: ";
    std::cin >> numThreads;

    HANDLE* threads = new HANDLE[numThreads];
    MarkerData* params = new MarkerData[numThreads];
    HANDLE startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    HANDLE* stopEvents = new HANDLE[numThreads];
    HANDLE* resumeEvents = new HANDLE[numThreads];
    HANDLE* exitEvents = new HANDLE[numThreads];

    std::vector<int> active_threads;
    int active_count = numThreads;

    for (int i = 0; i < numThreads; i++) {
        active_threads.push_back(i);
    }

    CRITICAL_SECTION cs;
    InitializeCriticalSection(&cs);

    for (int i = 0; i < numThreads; i++) {
        stopEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        resumeEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        exitEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        params[i].marker_index = i + 1;
        params[i].size = size;
        params[i].arr = array;
        params[i].startEvent = startEvent;
        params[i].stopEvent = stopEvents[i];
        params[i].resumeEvent = resumeEvents[i];
        params[i].exitEvent = exitEvents[i];
        params[i].cs = cs;
        threads[i] = CreateThread(NULL, 0, marker, &params[i], 0, NULL);
    }

    SetEvent(startEvent);
    while (active_count) {

        // wait for all markers to run
        WaitForMultipleObjects(numThreads, stopEvents, TRUE, INFINITE);

        // print array
        EnterCriticalSection(&cs);
        std::cout << "Array: ";
        for (int i = 0; i < size; i++) {
            std::cout << array[i] << " ";
        }
        std::cout << "\n";
        LeaveCriticalSection(&cs);

        
        // ask user which marker to stop
        int threadToStop;
        bool continue_reading = false;
        do {
            std::cout << "Enter marker thread number to stop: ";
            std::cin >> threadToStop;
            
            if (threadToStop <= 0 || threadToStop > numThreads) {
                std::cout << "There is no marker with this number, try again\n\n";
                continue_reading = true;
                continue;
            }
            else {
                continue_reading = false;
            }
            if (active_threads[threadToStop - 1] == -1) {
                std::cout << "This marker is already stopped, try again\n\n";
            }

        } while (continue_reading || active_threads[threadToStop - 1] == -1 );
        
        // tell it to exit
        SetEvent(exitEvents[threadToStop - 1]);

        // tell it to resume (and find out that it has to exit)
        SetEvent(resumeEvents[threadToStop - 1]);

        // delete it from active
        active_threads[threadToStop - 1] = -1;
        active_count--;

        // tell every active marker (NOT including the one to be exited) to resume
        // and reset their stop event

        for (int i = 0; i < active_threads.size(); i++) {
            if (active_threads[i] != -1) {
                ResetEvent(stopEvents[active_threads[i]]);
                SetEvent(resumeEvents[active_threads[i]]);
            }
            
        }

        // wait for marker to be exited, then close its handle
        WaitForSingleObject(threads[threadToStop - 1], INFINITE);
        CloseHandle(threads[threadToStop - 1]);

        // print updated array
        EnterCriticalSection(&cs);
        std::cout << "Updated Array: ";
        for (int i = 0; i < size; i++) {
            std::cout << array[i] << " ";
        }
        std::cout << "\n";
        LeaveCriticalSection(&cs);
    }

    DeleteCriticalSection(&cs);
    delete[] array;
    delete[] threads;
    delete[] params;
    delete[] stopEvents;
    delete[] resumeEvents;
    delete[] exitEvents;

    return 0;
}
