#pragma once
#include "common.h"
#include "gtest/gtest.h"
 // ���������� ��� ����� ������������ ����
 // ��� strncpy, strlen

// ������ ����� ��� ��������� Message
TEST(MessageTest, ConstructionAndData) {
    Message msg;
    ZeroMemory(msg.data, MAX_MESSAGE_LENGTH); // ��� � ����� ����

    const char* testData = "Hello";
    strncpy(msg.data, testData, MAX_MESSAGE_LENGTH - 1);
    // strncpy �� ������ ��������� null-����������, ���� �������� >= ����� �����������
    msg.data[MAX_MESSAGE_LENGTH - 1] = '\0'; // ����������� null-���������� ��� ������������

    ASSERT_STREQ(msg.data, "Hello"); // ���������� ��� C-������
    ASSERT_EQ(strlen(msg.data), 5);
}

TEST(MessageTest, MaxLengthHandling) {
    Message msg;
    ZeroMemory(msg.data, MAX_MESSAGE_LENGTH);

    const char* longData = "ThisIsAVeryLongMessageThatExceedsTwentyChars";
    strncpy(msg.data, longData, MAX_MESSAGE_LENGTH); // �������� ����� MAX_MESSAGE_LENGTH

    // ���� �������� ������ ���� >= MAX_MESSAGE_LENGTH, strncpy �� ������� '\0'.
    // �� ������ ���������, ��� ����� �� ����� �������� �� �������.
    // ��� �������� ������, ���� �� ������ ���� null-������������ � �������� 20 ��������:
    char expected[MAX_MESSAGE_LENGTH + 1] = { 0 };
    strncpy(expected, longData, MAX_MESSAGE_LENGTH);
    // expected[MAX_MESSAGE_LENGTH] ��� ����� '\0' ��-�� ������������� {0}

    // ���������� ������ MAX_MESSAGE_LENGTH ����
    ASSERT_EQ(strncmp(msg.data, expected, MAX_MESSAGE_LENGTH), 0);

    // ���� �� �����, ����� msg.data ������ ���� �������� C-������� ������ 20 ��������
    // (�.�. ��������� ������ MAX_MESSAGE_LENGTH-1 ������ ���� \0 ���� ��������� �������)
    Message msgSafe;
    ZeroMemory(msgSafe.data, MAX_MESSAGE_LENGTH);
    strncpy(msgSafe.data, longData, MAX_MESSAGE_LENGTH - 1);
    msgSafe.data[MAX_MESSAGE_LENGTH - 1] = '\0';

    char expectedSafe[MAX_MESSAGE_LENGTH] = { 0 }; // ����� ��� ������ 19 �������� + \0
    strncpy(expectedSafe, longData, MAX_MESSAGE_LENGTH - 1);
    // expectedSafe[MAX_MESSAGE_LENGTH - 1] ��� \0

    ASSERT_STREQ(msgSafe.data, expectedSafe);
    ASSERT_EQ(strlen(msgSafe.data), MAX_MESSAGE_LENGTH - 1);
}


// ������ ����� ��� FileHeader (���� �� ��� ���� ������)
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

// ����� ����� ��� Google Test (�� �����, ���� ���������� GTest::gtest_main)
// int main(int argc, char **argv) {
//   ::testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }