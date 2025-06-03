#pragma once
#include "common.h"
#include "gtest/gtest.h"
 // Подключаем наш общий заголовочный файл
 // для strncpy, strlen

// Пример теста для структуры Message
TEST(MessageTest, ConstructionAndData) {
    Message msg;
    ZeroMemory(msg.data, MAX_MESSAGE_LENGTH); // Как в вашем коде

    const char* testData = "Hello";
    strncpy(msg.data, testData, MAX_MESSAGE_LENGTH - 1);
    // strncpy не всегда добавляет null-терминатор, если источник >= длины копирования
    msg.data[MAX_MESSAGE_LENGTH - 1] = '\0'; // Гарантируем null-терминатор для безопасности

    ASSERT_STREQ(msg.data, "Hello"); // Сравниваем как C-строки
    ASSERT_EQ(strlen(msg.data), 5);
}

TEST(MessageTest, MaxLengthHandling) {
    Message msg;
    ZeroMemory(msg.data, MAX_MESSAGE_LENGTH);

    const char* longData = "ThisIsAVeryLongMessageThatExceedsTwentyChars";
    strncpy(msg.data, longData, MAX_MESSAGE_LENGTH); // Копируем ровно MAX_MESSAGE_LENGTH

    // Если исходная строка была >= MAX_MESSAGE_LENGTH, strncpy не добавит '\0'.
    // Мы должны убедиться, что буфер не будет прочитан за пределы.
    // Для проверки вывода, если он должен быть null-терминирован в пределах 20 символов:
    char expected[MAX_MESSAGE_LENGTH + 1] = { 0 };
    strncpy(expected, longData, MAX_MESSAGE_LENGTH);
    // expected[MAX_MESSAGE_LENGTH] уже будет '\0' из-за инициализации {0}

    // Сравниваем первые MAX_MESSAGE_LENGTH байт
    ASSERT_EQ(strncmp(msg.data, expected, MAX_MESSAGE_LENGTH), 0);

    // Если мы хотим, чтобы msg.data всегда была валидной C-строкой внутри 20 символов
    // (т.е. последний символ MAX_MESSAGE_LENGTH-1 должен быть \0 если сообщение длинное)
    Message msgSafe;
    ZeroMemory(msgSafe.data, MAX_MESSAGE_LENGTH);
    strncpy(msgSafe.data, longData, MAX_MESSAGE_LENGTH - 1);
    msgSafe.data[MAX_MESSAGE_LENGTH - 1] = '\0';

    char expectedSafe[MAX_MESSAGE_LENGTH] = { 0 }; // буфер для первых 19 символов + \0
    strncpy(expectedSafe, longData, MAX_MESSAGE_LENGTH - 1);
    // expectedSafe[MAX_MESSAGE_LENGTH - 1] уже \0

    ASSERT_STREQ(msgSafe.data, expectedSafe);
    ASSERT_EQ(strlen(msgSafe.data), MAX_MESSAGE_LENGTH - 1);
}


// Пример теста для FileHeader (если бы там была логика)
TEST(FileHeaderTest, InitialValues) {
    FileHeader header;
    header.writeIndex = 0;
    header.readIndex = 0;
    header.maxMessages = 10;
    header.messageCount = 0;

    ASSERT_EQ(header.writeIndex, 0);
    ASSERT_EQ(header.readIndex, 0);
    ASSERT_EQ(header.maxMessages, 10);
    ASSERT_EQ(header.messageCount, 0);
}

// Точка входа для Google Test (не нужна, если используем GTest::gtest_main)
// int main(int argc, char **argv) {
//   ::testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }