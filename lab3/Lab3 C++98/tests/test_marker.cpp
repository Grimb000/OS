#include <gtest/gtest.h>
#include <windows.h>
#include <vector>
#include "marker.h"
#include "MarkerData.h"

class MarkerTest : public ::testing::Test {
protected:
    MarkerData data;
    CRITICAL_SECTION cs;

    void SetUp() override {
        data.size = 10;
        data.arr = new int[data.size]();
        data.marker_index = 1;

        InitializeCriticalSection(&cs);
        data.cs = cs;

        data.startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        data.stopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        data.resumeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        data.exitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    void TearDown() override {
        DeleteCriticalSection(&cs);
        delete[] data.arr;

        CloseHandle(data.startEvent);
        CloseHandle(data.stopEvent);
        CloseHandle(data.resumeEvent);
        CloseHandle(data.exitEvent);
    }
};

TEST_F(MarkerTest, MarksAtLeastOneElement) {
    HANDLE hThread = CreateThread(NULL, 0, marker, &data, 0, NULL);
    ASSERT_NE(hThread, nullptr);

    SetEvent(data.startEvent);
    Sleep(50);
    SetEvent(data.stopEvent);
    Sleep(50);

    bool hasMarked = false;
    for (int i = 0; i < data.size; ++i) {
        if (data.arr[i] == data.marker_index) {
            hasMarked = true;
            break;
        }
    }

    SetEvent(data.exitEvent);
    SetEvent(data.resumeEvent);

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    EXPECT_TRUE(hasMarked);
}

TEST_F(MarkerTest, ClearsMarksOnExit) {
    HANDLE hThread = CreateThread(NULL, 0, marker, &data, 0, NULL);
    ASSERT_NE(hThread, nullptr);

    SetEvent(data.startEvent);
    Sleep(50);
    SetEvent(data.stopEvent);
    Sleep(50);
    SetEvent(data.exitEvent);
    SetEvent(data.resumeEvent);

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    for (int i = 0; i < data.size; ++i) {
        EXPECT_EQ(data.arr[i], 0);
    }
}

TEST_F(MarkerTest, SkipsMarkedElements) {
    data.arr[3] = 99;

    HANDLE hThread = CreateThread(NULL, 0, marker, &data, 0, NULL);
    ASSERT_NE(hThread, nullptr);

    SetEvent(data.startEvent);
    Sleep(50);
    SetEvent(data.stopEvent);
    Sleep(50);

    EXPECT_EQ(data.arr[3], 99);

    SetEvent(data.exitEvent);
    SetEvent(data.resumeEvent);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
}

