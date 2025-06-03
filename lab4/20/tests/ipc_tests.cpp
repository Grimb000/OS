#include "gtest/gtest.h"
#include "common.h" // ����������������� common
#include "utils.h"  // ��� WinApiException, ���� ����� ��������������
#include <string>
#include <cstring> // ��� strncpy, strlen, strnlen

// ������ ����� ��� ��������� Message
TEST(MessageModernTest, ConstructionAndData) {
    Message msg{}; // Value initialization (data is zero-filled)

    const char* testData = "Hello Modern";
    // ���������� ����������� � C++ ����� (���� ��� POD char[] � strncpy_s ���������)
    std::string s_testData = testData;
    s_testData.copy(msg.data, s_testData.length());
    // �������� � null-����������, ���� ������ ������ ������
    if (s_testData.length() < MAX_MESSAGE_LENGTH) {
        msg.data[s_testData.length()] = '\0';
    }
    else {
        msg.data[MAX_MESSAGE_LENGTH - 1] = '\0'; // �����������, ���� ������ ��������� ���� �����
    }

    ASSERT_STREQ(msg.data, "Hello Modern");
    ASSERT_EQ(strlen(msg.data), strlen("Hello Modern"));
}

TEST(MessageModernTest, MaxLengthHandling) {
    Message msg{};
    const std::string longData = "ThisIsAVeryLongMessageThatExceedsTheTwentyCharLimitForSure";

    // �������� �� ����� MAX_MESSAGE_LENGTH - 1 ��������, ����� �������� ����� ��� '\0'
    longData.copy(msg.data, MAX_MESSAGE_LENGTH - 1);
    msg.data[MAX_MESSAGE_LENGTH - 1] = '\0'; // ����������� null-����������

    char expected[MAX_MESSAGE_LENGTH];
    strncpy(expected, longData.c_str(), MAX_MESSAGE_LENGTH - 1);
    expected[MAX_MESSAGE_LENGTH - 1] = '\0';

    ASSERT_STREQ(msg.data, expected);
    ASSERT_EQ(strlen(msg.data), MAX_MESSAGE_LENGTH - 1);
}

// ���� ��� FileHeader (�������)
TEST(FileHeaderModernTest, InitialValues) {
    FileHeader header{}; // Value initialization
    header.maxMessages = 10;

    ASSERT_EQ(header.writeIndex, 0);
    ASSERT_EQ(header.readIndex, 0);
    ASSERT_EQ(header.maxMessages, 10);
    ASSERT_EQ(header.messageCount, 0);
}

// ����� �������� ���� ��� WinApiException, ���� ���� �������
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