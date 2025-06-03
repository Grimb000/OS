#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>  
#include <windows.h>
#include "common.h"  
void PrintError(const char* message) {
	std::cerr << message << " GLE: " << GetLastError() << std::endl;
}

int main() {
	std::string fileName;
	int numRecords;
	int numSenders;

	std::cout << "Enter binary file name: ";
	std::cin >> fileName;
	std::cout << "Enter number of records (queue capacity): ";
	std::cin >> numRecords;

	if (numRecords <= 0) {
		std::cerr << "Number of records must be positive." << std::endl;
		return 1;
	}

	HANDLE hFile = CreateFile(
		fileName.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		PrintError("Failed to create binary file.");
		return 1;
	}

	FileHeader header;
	header.writeIndex = 0;
	header.readIndex = 0;
	header.maxMessages = static_cast<UINT>(numRecords);
	header.messageCount = 0;

	DWORD bytesWritten;
	if (!WriteFile(hFile, &header, sizeof(FileHeader), &bytesWritten, NULL) || bytesWritten != sizeof(FileHeader)) {
		PrintError("Failed to write file header.");
		CloseHandle(hFile);
		return 1;
	}

	Message emptyMsg;
	ZeroMemory(emptyMsg.data, MAX_MESSAGE_LENGTH);
	for (int i = 0; i < numRecords; ++i) {
		if (!WriteFile(hFile, &emptyMsg, sizeof(Message), &bytesWritten, NULL) || bytesWritten != sizeof(Message)) {
			PrintError("Failed to initialize message slot in file.");
			CloseHandle(hFile);
			return 1;
		}
	}
	std::cout << "Binary file '" << fileName << "' created with " << numRecords << " records." << std::endl;

	HANDLE hMutex = CreateMutex(NULL, FALSE, SHARED_FILE_MUTEX_NAME);
	if (hMutex == NULL) {
		PrintError("Failed to create mutex.");
		CloseHandle(hFile);
		return 1;
	}

	HANDLE hSemaphoreEmpty = CreateSemaphore(NULL, numRecords, numRecords, SEMAPHORE_EMPTY_NAME);
	if (hSemaphoreEmpty == NULL) {
		PrintError("Failed to create empty semaphore.");
		CloseHandle(hMutex);
		CloseHandle(hFile);
		return 1;
	}

	HANDLE hSemaphoreFull = CreateSemaphore(NULL, 0, numRecords, SEMAPHORE_FULL_NAME);
	if (hSemaphoreFull == NULL) {
		PrintError("Failed to create full semaphore.");
		CloseHandle(hSemaphoreEmpty);
		CloseHandle(hMutex);
		CloseHandle(hFile);
		return 1;
	}

	std::cout << "Enter number of Sender processes (max " << MAX_SENDERS << "): ";
	std::cin >> numSenders;

	if (numSenders <= 0 || numSenders > MAX_SENDERS) {
		std::cerr << "Invalid number of Senders." << std::endl;
		CloseHandle(hSemaphoreFull);
		CloseHandle(hSemaphoreEmpty);
		CloseHandle(hMutex);
		CloseHandle(hFile);
		return 1;
	}

	std::vector<HANDLE> senderProcessHandles(numSenders);
	std::vector<HANDLE> senderReadyEventHandles(numSenders);
	std::vector<STARTUPINFO> si(numSenders);
	std::vector<PROCESS_INFORMATION> pi(numSenders);

	for (int i = 0; i < numSenders; ++i) {
		ZeroMemory(&si[i], sizeof(STARTUPINFO));
		si[i].cb = sizeof(STARTUPINFO);
		ZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));

		std::ostringstream eventNameStream;
		eventNameStream << SENDER_READY_EVENT_PREFIX << i;
		std::string senderReadyEventName = eventNameStream.str();

		senderReadyEventHandles[i] = CreateEvent(NULL, TRUE, FALSE, senderReadyEventName.c_str());          if (senderReadyEventHandles[i] == NULL) {
			PrintError("Failed to create sender ready event.");
			for (int j = 0; j < i; ++j) {
				CloseHandle(senderReadyEventHandles[j]);
				TerminateProcess(pi[j].hProcess, 1);                  CloseHandle(pi[j].hProcess);
				CloseHandle(pi[j].hThread);
			}
			CloseHandle(hSemaphoreFull);
			CloseHandle(hSemaphoreEmpty);
			CloseHandle(hMutex);
			CloseHandle(hFile);
			return 1;
		}

		std::string commandLine = "Sender.exe " + fileName + " " + senderReadyEventName;
		char* cmdLineCstr = new char[commandLine.length() + 1];
		strcpy(cmdLineCstr, commandLine.c_str());


		if (!CreateProcess(
			NULL, cmdLineCstr, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i])) {
			PrintError("Failed to create Sender process.");
			delete[] cmdLineCstr;
			CloseHandle(senderReadyEventHandles[i]);
			PrintError("Failed to create sender ready event.");
			for (int j = 0; j < i; ++j) {
				CloseHandle(senderReadyEventHandles[j]);
				TerminateProcess(pi[j].hProcess, 1);                  
				CloseHandle(pi[j].hProcess);
				CloseHandle(pi[j].hThread);
			}
			CloseHandle(hSemaphoreFull);
			CloseHandle(hSemaphoreEmpty);
			CloseHandle(hMutex);
			CloseHandle(hFile);
			return 1;
		}
		delete[] cmdLineCstr;
		senderProcessHandles[i] = pi[i].hProcess;          
		CloseHandle(pi[i].hThread);          
		std::cout << "Sender process " << i << " launched." << std::endl;
	}

	std::cout << "Waiting for all Senders to be ready..." << std::endl;
	if (numSenders > 0) {
		DWORD waitResult = WaitForMultipleObjects(numSenders, &senderReadyEventHandles[0], TRUE, INFINITE);          
		if (waitResult >= WAIT_OBJECT_0 && waitResult < WAIT_OBJECT_0 + numSenders) {
			std::cout << "All Senders are ready." << std::endl;
		}
		else {
			PrintError("Failed waiting for sender ready events.");
			return 1;
		}
	}


	for (int i = 0; i < numSenders; ++i) {
		CloseHandle(senderReadyEventHandles[i]);
	}
	senderReadyEventHandles.clear();


	std::string command;
	while (true) {
		std::cout << "\nEnter command ('read' or 'exit'): ";
		std::cin >> command;

		if (command == "read") {
			std::cout << "Waiting for a message..." << std::endl;
			DWORD waitRes = WaitForSingleObject(hSemaphoreFull, INFINITE);
			if (waitRes != WAIT_OBJECT_0) {
				PrintError("Failed to wait for full semaphore in Receiver.");
				continue;
			}

			waitRes = WaitForSingleObject(hMutex, INFINITE);
			if (waitRes != WAIT_OBJECT_0) {
				PrintError("Failed to wait for mutex in Receiver.");
				ReleaseSemaphore(hSemaphoreFull, 1, NULL);                  continue;
			}

			if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
				PrintError("Receiver: SetFilePointer to header failed.");
				ReleaseMutex(hMutex);
				ReleaseSemaphore(hSemaphoreFull, 1, NULL);
				continue;
			}
			DWORD bytesRead;
			if (!ReadFile(hFile, &header, sizeof(FileHeader), &bytesRead, NULL) || bytesRead != sizeof(FileHeader)) {
				PrintError("Receiver: Failed to read header.");
				ReleaseMutex(hMutex);
				ReleaseSemaphore(hSemaphoreFull, 1, NULL);
				continue;
			}

			if (header.messageCount == 0) {
				std::cout << "File is empty (unexpected behavior with semaphore)." << std::endl;
				ReleaseMutex(hMutex);
			}
			else {
				long messageOffset = sizeof(FileHeader) + header.readIndex * sizeof(Message);
				if (SetFilePointer(hFile, messageOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
					PrintError("Receiver: SetFilePointer to message failed.");
					ReleaseMutex(hMutex);
					ReleaseSemaphore(hSemaphoreFull, 1, NULL);
					continue;
				}

				Message receivedMsg;
				if (!ReadFile(hFile, &receivedMsg, sizeof(Message), &bytesRead, NULL) || bytesRead != sizeof(Message)) {
					PrintError("Receiver: Failed to read message.");
				}
				else {
					std::cout << "Received message: ";
					std::cout.write(receivedMsg.data, strnlen(receivedMsg.data, MAX_MESSAGE_LENGTH));
					std::cout << std::endl;

					header.readIndex = (header.readIndex + 1) % header.maxMessages;
					header.messageCount--;

					if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
						PrintError("Receiver: SetFilePointer to header for update failed.");
					}
					else {
						if (!WriteFile(hFile, &header, sizeof(FileHeader), &bytesWritten, NULL) || bytesWritten != sizeof(FileHeader)) {
							PrintError("Receiver: Failed to write updated header.");
						}
					}
					ReleaseSemaphore(hSemaphoreEmpty, 1, NULL);
				}
			}
			ReleaseMutex(hMutex);

		}
		else if (command == "exit") {
			std::cout << "Receiver exiting..." << std::endl;
			break;
		}
		else {
			std::cout << "Unknown command." << std::endl;
		}
	}

	std::cout << "Closing handles and terminating Senders..." << std::endl;
	for (size_t i = 0; i < senderProcessHandles.size(); ++i) {
		if (senderProcessHandles[i] != NULL) {
			TerminateProcess(senderProcessHandles[i], 0);
			CloseHandle(senderProcessHandles[i]);
		}
	}

	CloseHandle(hMutex);
	CloseHandle(hSemaphoreEmpty);
	CloseHandle(hSemaphoreFull);
	CloseHandle(hFile);

	std::cout << "Receiver finished." << std::endl;
	return 0;
}