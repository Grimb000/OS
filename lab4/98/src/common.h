#pragma once
#ifndef COMMON_H
#define COMMON_H

#include <windows.h> 
const int MAX_MESSAGE_LENGTH = 20;
const int MAX_SENDERS = 10;
struct FileHeader {
	UINT writeIndex;
	UINT readIndex;
	UINT maxMessages;
	UINT messageCount;
};

struct Message {
	char data[MAX_MESSAGE_LENGTH];
};

const char* SHARED_FILE_MUTEX_NAME = "Global\\MessageFileMutex";
const char* SEMAPHORE_EMPTY_NAME = "Global\\MessageFileEmptySemaphore";
const char* SEMAPHORE_FULL_NAME = "Global\\MessageFileFullSemaphore";
const char* SENDER_READY_EVENT_PREFIX = "Global\\SenderReadyEvent_";
#endif 