#include "gtest/gtest.h"
#include "common.h" // Модернизированный common
#include "utils.h"  // Для WinApiException, если нужно протестировать
#include <string>
#include <cstring> // для strncpy, strlen, strnlen

// Пример теста для структуры Message
TEST(MessageModernTest, ConstructionAndData) {
    Message msg{}; // Value initialization (data is zero-filled)

    const char* testData = "Hello Modern";
    // Безопасное копирование в C++ стиле (хотя для POD char[] и strncpy_s нормально)
    std::string s_testData = testData;
    s_testData.copy(msg.data, s_testData.length());
    // Убедимся в null-терминации, если строка короче буфера
    if (s_testData.length() < MAX_MESSAGE_LENGTH) {
        msg.data[s_testData.length()] = '\0';
    }
    else {
        msg.data[MAX_MESSAGE_LENGTH - 1] = '\0'; // Гарантируем, если строка заполнила весь буфер
    }

    ASSERT_STREQ(msg.data, "Hello Modern");
    ASSERT_EQ(strlen(msg.data), strlen("Hello Modern"));
}

TEST(MessageModernTest, MaxLengthHandling) {
    Message msg{};
    const std::string longData = "ThisIsAVeryLongMessageThatExceedsTheTwentyCharLimitForSure";

    // Копируем не более MAX_MESSAGE_LENGTH - 1 символов, чтобы оставить место для '\0'
    longData.copy(msg.data, MAX_MESSAGE_LENGTH - 1);
    msg.data[MAX_MESSAGE_LENGTH - 1] = '\0'; // Гарантируем null-терминацию

    char expected[MAX_MESSAGE_LENGTH];
    strncpy(expected, longData.c_str(), MAX_MESSAGE_LENGTH - 1);
    expected[MAX_MESSAGE_LENGTH - 1] = '\0';

    ASSERT_STREQ(msg.data, expected);
    ASSERT_EQ(strlen(msg.data), MAX_MESSAGE_LENGTH - 1);
}

// Тест для FileHeader (простой)
TEST(FileHeaderModernTest, InitialValues) {
    FileHeader header{}; // Value initialization
    header.maxMessages = 10;

    ASSERT_EQ(header.writeIndex, 0);
    ASSERT_EQ(header.readIndex, 0);
    ASSERT_EQ(header.maxMessages, 10);
    ASSERT_EQ(header.messageCount, 0);
}

// Можно добавить тест для WinApiException, если есть желание
TEST(UtilsModernTest, WinApiExceptionMessage) {
    try {
        throw WinApiException("Test exception", 123);
    }
    catch (const WinApiException& e) {
        std::string expected_msg = "Test exception (Windows Error Code: 123)";
        ASSERT_STREQ(e.what(), expected_msg.c_str());
        ASSERT_EQ(e.get_error_code(), 123);
    }
    catch (...) {
        FAIL() << "Expected WinApiException";
    }
}