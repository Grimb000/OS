#ifndef COMMON_MODERN_H
#define COMMON_MODERN_H
#define NOMINMAX
#include <windows.h> // Для UINT и других типов WinAPI
#include <string>    // Для std::string
#include <array>     // Для std::array, если решим использовать

// Константы с использованием constexpr
constexpr int MAX_MESSAGE_LENGTH = 20;
constexpr int MAX_SENDERS_ALLOWED = 10; // Переименовано для ясности

// Структура для заголовка в бинарном файле (остается POD для простоты I/O)
struct FileHeader {
    UINT writeIndex;
    UINT readIndex;
    UINT maxMessages;
    UINT messageCount;
};

// Структура для сообщения (остается POD)
struct Message {
    char data[MAX_MESSAGE_LENGTH]; // Для бинарной совместимости и простоты I/O
    // std::array<char, MAX_MESSAGE_LENGTH> data; был бы альтернативой
};

// Имена объектов синхронизации (используем std::string для удобства)
// Префикс "Global\\" для глобальных объектов ядра
const std::string SHARED_FILE_MUTEX_NAME_MODERN = "Global\\ModernMessageFileMutex_v2";
const std::string SEMAPHORE_EMPTY_NAME_MODERN = "Global\\ModernMessageFileEmptySemaphore_v2";
const std::string SEMAPHORE_FULL_NAME_MODERN = "Global\\ModernMessageFileFullSemaphore_v2";
const std::string SENDER_READY_EVENT_PREFIX_MODERN = "Global\\ModernSenderReadyEvent_v2_";

#endif // COMMON_MODERN_H