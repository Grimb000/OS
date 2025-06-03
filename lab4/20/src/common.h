#ifndef COMMON_MODERN_H
#define COMMON_MODERN_H
#define NOMINMAX
#include <windows.h> // ��� UINT � ������ ����� WinAPI
#include <string>    // ��� std::string
#include <array>     // ��� std::array, ���� ����� ������������

// ��������� � �������������� constexpr
constexpr int MAX_MESSAGE_LENGTH = 20;
constexpr int MAX_SENDERS_ALLOWED = 10; // ������������� ��� �������

// ��������� ��� ��������� � �������� ����� (�������� POD ��� �������� I/O)
struct FileHeader {
    UINT writeIndex;
    UINT readIndex;
    UINT maxMessages;
    UINT messageCount;
};

// ��������� ��� ��������� (�������� POD)
struct Message {
    char data[MAX_MESSAGE_LENGTH]; // ��� �������� ������������� � �������� I/O
    // std::array<char, MAX_MESSAGE_LENGTH> data; ��� �� �������������
};

// ����� �������� ������������� (���������� std::string ��� ��������)
// ������� "Global\\" ��� ���������� �������� ����
const std::string SHARED_FILE_MUTEX_NAME_MODERN = "Global\\ModernMessageFileMutex_v2";
const std::string SEMAPHORE_EMPTY_NAME_MODERN = "Global\\ModernMessageFileEmptySemaphore_v2";
const std::string SEMAPHORE_FULL_NAME_MODERN = "Global\\ModernMessageFileFullSemaphore_v2";
const std::string SENDER_READY_EVENT_PREFIX_MODERN = "Global\\ModernSenderReadyEvent_v2_";

#endif // COMMON_MODERN_H