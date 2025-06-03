#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include "common.h"  
void PrintError(const char* message) {
	std::cerr << message << " GLE: " << GetLastError() << std::endl;
}

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Usage: Sender.exe <fileName> <readyEventName>" << std::endl;
		return 1;
	}

	std::string fileName = argv[1];
	std::string readyEventName = argv[2];

	HANDLE hFile = CreateFile(
		fileName.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		PrintError("Sender: Failed to open binary file.");
		return 1;
	}

	HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, SHARED_FILE_MUTEX_NAME);
	if (hMutex == NULL) {
		PrintError("Sender: Failed to open mutex.");
		CloseHandle(hFile);
		return 1;
	}

	HANDLE hSemaphoreEmpty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_EMPTY_NAME);
	if (hSemaphoreEmpty == NULL) {
		PrintError("Sender: Failed to open empty semaphore.");
		CloseHandle(hMutex);
		CloseHandle(hFile);
		return 1;
	}

	HANDLE hSemaphoreFull = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_FULL_NAME);
	if (hSemaphoreFull == NULL) {
		PrintError("Sender: Failed to open full semaphore.");
		CloseHandle(hSemaphoreEmpty);
		CloseHandle(hMutex);
		CloseHandle(hFile);
		return 1;
	}

	HANDLE hReadyEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, readyEventName.c_str());
	if (hReadyEvent == NULL) {
		PrintError("Sender: Failed to open ready event.");
		CloseHandle(hSemaphoreFull);
		CloseHandle(hSemaphoreEmpty);
		CloseHandle(hMutex);
		CloseHandle(hFile);
		return 1;
	}

	if (!SetEvent(hReadyEvent)) {
		PrintError("Sender: Failed to set ready event.");
	}
	else {
		std::cout << "Sender is ready, signaled event " << readyEventName << std::endl;
	}
	CloseHandle(hReadyEvent);

	std::string command;
	std::string messageText;
	while (true) {
		std::cout << "\nSender: Enter command ('send <message_text>' or 'exit'): ";
		std::cin >> command;

		if (command == "send") {
			char msgBuffer[MAX_MESSAGE_LENGTH + 1];              std::cin.getline(msgBuffer, sizeof(msgBuffer));
			messageText = (msgBuffer[0] == ' ' ? msgBuffer + 1 : msgBuffer);


			if (messageText.length() == 0) {
				std::cout << "Sender: Message cannot be empty." << std::endl;
				continue;
			}
			if (messageText.length() > MAX_MESSAGE_LENGTH) {
				std::cout << "Sender: Message too long (max " << MAX_MESSAGE_LENGTH << " chars). Truncating." << std::endl;
				messageText = messageText.substr(0, MAX_MESSAGE_LENGTH);
			}

			std::cout << "Sender: Waiting for an empty slot..." << std::endl;
			DWORD waitRes = WaitForSingleObject(hSemaphoreEmpty, INFINITE);
			if (waitRes != WAIT_OBJECT_0) {
				PrintError("Sender: Failed to wait for empty semaphore.");
				continue;
			}

			waitRes = WaitForSingleObject(hMutex, INFINITE);
			if (waitRes != WAIT_OBJECT_0) {
				PrintError("Sender: Failed to wait for mutex.");
				ReleaseSemaphore(hSemaphoreEmpty, 1, NULL);                  continue;
			}

			FileHeader header;
			if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
				PrintError("Sender: SetFilePointer to header failed.");
				ReleaseMutex(hMutex);
				ReleaseSemaphore(hSemaphoreEmpty, 1, NULL);
				continue;
			}
			DWORD bytesReadOrWritten;
			if (!ReadFile(hFile, &header, sizeof(FileHeader), &bytesReadOrWritten, NULL) || bytesReadOrWritten != sizeof(FileHeader)) {
				PrintError("Sender: Failed to read header.");
				ReleaseMutex(hMutex);
				ReleaseSemaphore(hSemaphoreEmpty, 1, NULL);
				continue;
			}

			if (header.messageCount >= header.maxMessages) {
				std::cerr << "Sender: File is full (unexpected behavior with semaphore)." << std::endl;
				ReleaseMutex(hMutex);
			}
			else {
				Message msgToSend;
				ZeroMemory(msgToSend.data, MAX_MESSAGE_LENGTH);                  strncpy(msgToSend.data, messageText.c_str(), MAX_MESSAGE_LENGTH - 1);                                                    if (messageText.length() < MAX_MESSAGE_LENGTH) {
					msgToSend.data[messageText.length()] = '\0';
				}
				else {
					msgToSend.data[MAX_MESSAGE_LENGTH - 1] = '\0';
				}


				long messageOffset = sizeof(FileHeader) + header.writeIndex * sizeof(Message);
				if (SetFilePointer(hFile, messageOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
					PrintError("Sender: SetFilePointer to message slot failed.");
				}
				else {
					if (!WriteFile(hFile, &msgToSend, sizeof(Message), &bytesReadOrWritten, NULL) || bytesReadOrWritten != sizeof(Message)) {
						PrintError("Sender: Failed to write message.");
					}
					else {
						header.writeIndex = (header.writeIndex + 1) % header.maxMessages;
						header.messageCount++;

						if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
							PrintError("Sender: SetFilePointer to header for update failed.");
						}
						else {
							if (!WriteFile(hFile, &header, sizeof(FileHeader), &bytesReadOrWritten, NULL) || bytesReadOrWritten != sizeof(FileHeader)) {
								PrintError("Sender: Failed to write updated header.");
							}
							else {
								std::cout << "Sender: Message '" << messageText << "' sent." << std::endl;
								ReleaseSemaphore(hSemaphoreFull, 1, NULL);
							}
						}
					}
				}
			}
			ReleaseMutex(hMutex);

		}
		else if (command == "exit") {
			std::cout << "Sender exiting..." << std::endl;
			break;
		}
		else {
			std::cin.ignore(std::numeric_limits<std::streamsize>::infinity(), '\n');
			std::cout << "Sender: Unknown command." << std::endl;
		}
	}

	CloseHandle(hSemaphoreFull);
	CloseHandle(hSemaphoreEmpty);
	CloseHandle(hMutex);
	CloseHandle(hFile);

	std::cout << "Sender finished." << std::endl;
	return 0;
}