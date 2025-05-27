#include "marker.h"
#include <unordered_set>
#include <iostream>
#include <mutex>
#include "MarkerData.h"

int main() {
    int size, numThreads;
    std::cout << "Enter array size: ";
    std::cin >> size;

    int* array = new int[size]();

    std::cout << "Enter number of marker threads: ";
    std::cin >> numThreads;

    std::vector<HANDLE> threads(numThreads);
    std::vector<MarkerData> params(numThreads);
    HANDLE startEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    std::vector<HANDLE> stopEvents(numThreads);
    std::vector<HANDLE> resumeEvents(numThreads);
    std::vector<HANDLE> exitEvents(numThreads);

    std::unordered_set<int> active_threads;
    for (int i = 0; i < numThreads; i++)
        active_threads.insert(i);

    std::mutex mtx;

    for (int i = 0; i < numThreads; i++) {
        stopEvents[i] = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        resumeEvents[i] = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        exitEvents[i] = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        //MarkerData md(i + 1, size, array.data(), startEvent, stopEvents[i], resumeEvents[i], exitEvents[i], mtx);

        params[i] = { i + 1, size, array, startEvent, stopEvents[i], resumeEvents[i], exitEvents[i], &mtx };
        threads[i] = CreateThread(nullptr, 0, marker, &params[i], 0, nullptr);
    }

    SetEvent(startEvent);
    while (!active_threads.empty()) {

        
        WaitForMultipleObjects(numThreads, stopEvents.data(), TRUE, INFINITE);

        // print array
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Array: ";
            for (int i = 0; i < size; ++i) {
                std::cout << array[i] << " ";
            }
            std::cout << "\n";
        }

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
            if (active_threads.find(threadToStop - 1) == active_threads.end()) {
                std::cout << "This marker is already stopped, try again\n\n";
            }

        } while (continue_reading || active_threads.find(threadToStop - 1) == active_threads.end());

        // tell it to exit
        SetEvent(exitEvents[threadToStop - 1]);

        // tell it to resume (and find out that it has to exit)
        SetEvent(resumeEvents[threadToStop - 1]);

        // delete it from active
        active_threads.erase(threadToStop - 1);

        // tell every active marker (NOT including the one to be exited) to resume
        // and reset their stop events
        for (int i : active_threads) {
            ResetEvent(stopEvents[i]);
            SetEvent(resumeEvents[i]);
        }

        // wait for marker to be exited, then close its handle
        WaitForSingleObject(threads[threadToStop - 1], INFINITE);
        CloseHandle(threads[threadToStop - 1]);

        // print updated array
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << "Updated Array: ";
            for (int i = 0; i < size; ++i) {
                std::cout << array[i] << " ";
            }
            std::cout << "\n";
        }
    }
    
    std::string aboba52;
    std::cin >> aboba52;
    delete[] array;
    
    return 0;
}
