#include <iostream>
#include <string>
#include <vector>
#include <sstream>   // Для std::ostringstream
#include <stdexcept> // Для std::invalid_argument

#include "common.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Sender: Usage: " << argv[0] << " <fileName> <readyEventName>" << std::endl;
        return 1;
    }

    try {
        std::string file_name_str = argv[1];
        std::string ready_event_name_str = argv[2];

        unique_handle h_file(CreateFileA(
            file_name_str.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
        ));
        if (h_file.get() == INVALID_HANDLE_VALUE) {
            throw WinApiException("Sender: Failed to open binary file '" + file_name_str + "'");
        }

        unique_handle mutex(OpenMutexA(MUTEX_ALL_ACCESS, FALSE, SHARED_FILE_MUTEX_NAME_MODERN.c_str()));
        if (!mutex) throw WinApiException("Sender: Failed to open mutex");

        unique_handle sem_empty(OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_EMPTY_NAME_MODERN.c_str()));
        if (!sem_empty) throw WinApiException("Sender: Failed to open empty semaphore");

        unique_handle sem_full(OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, SEMAPHORE_FULL_NAME_MODERN.c_str()));
        if (!sem_full) throw WinApiException("Sender: Failed to open full semaphore");

        unique_handle ready_event(OpenEventA(EVENT_MODIFY_STATE, FALSE, ready_event_name_str.c_str()));
        if (!ready_event) throw WinApiException("Sender: Failed to open ready event '" + ready_event_name_str + "'");

        if (!SetEvent(ready_event.get())) {
            throw WinApiException("Sender: Failed to set ready event");
        }
        std::cout << "Sender: Ready. Signaled event " << ready_event_name_str << std::endl;
        // ready_event unique_handle закроется автоматически, когда больше не будет нужен

        std::string line;
        std::string command;
        std::string message_text;

        while (true) {
            std::cout << "\nSender: Enter command ('send <message>' or 'exit'): ";
            std::getline(std::cin, line); // Читаем всю строку

            std::istringstream iss(line);
            iss >> command; // Извлекаем первое слово как команду

            if (command == "send") {
                // Извлекаем остаток строки как сообщение
                // std::ws убирает ведущие пробелы перед сообщением
                std::getline(iss >> std::ws, message_text);

                if (message_text.empty()) {
                    std::cout << "Sender: Message cannot be empty." << std::endl;
                    continue;
                }
                if (message_text.length() > MAX_MESSAGE_LENGTH) {
                    std::cout << "Sender: Message too long (max " << MAX_MESSAGE_LENGTH
                        << " chars). Truncating." << std::endl;
                    message_text.resize(MAX_MESSAGE_LENGTH);
                }

                std::cout << "Sender: Waiting for an empty slot..." << std::endl;
                if (WaitForSingleObject(sem_empty.get(), INFINITE) != WAIT_OBJECT_0) {
                    throw WinApiException("Sender: Failed to wait for empty semaphore");
                }
                if (WaitForSingleObject(mutex.get(), INFINITE) != WAIT_OBJECT_0) {
                    ReleaseSemaphore(sem_empty.get(), 1, NULL); // Откатить
                    throw WinApiException("Sender: Failed to wait for mutex");
                }

                FileHeader current_header;
                DWORD bytes_op;

                if (SetFilePointer(h_file.get(), 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
                    ReleaseMutex(mutex.get()); ReleaseSemaphore(sem_empty.get(), 1, NULL);
                    throw WinApiException("Sender: Send - SetFilePointer to header failed");
                }
                if (!ReadFile(h_file.get(), &current_header, sizeof(FileHeader), &bytes_op, NULL) || bytes_op != sizeof(FileHeader)) {
                    ReleaseMutex(mutex.get()); ReleaseSemaphore(sem_empty.get(), 1, NULL);
                    throw WinApiException("Sender: Send - Failed to read header");
                }

                // Семафор Empty гарантирует, что current_header.messageCount < current_header.maxMessages
                Message msg_to_send = {}; // Zero-initialize data
                strncpy_s(msg_to_send.data, MAX_MESSAGE_LENGTH, message_text.c_str(), message_text.length());
                // strncpy_s гарантирует null-терминацию, если буфер достаточен.
                // Или можно сделать вручную для большей ясности или если strncpy_s недоступна/нежелательна:
                // message_text.copy(msg_to_send.data, message_text.length());
                // if (message_text.length() < MAX_MESSAGE_LENGTH) msg_to_send.data[message_text.length()] = '\0';


                long message_offset = sizeof(FileHeader) + current_header.writeIndex * sizeof(Message);
                if (SetFilePointer(h_file.get(), message_offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
                    ReleaseMutex(mutex.get()); ReleaseSemaphore(sem_empty.get(), 1, NULL);
                    throw WinApiException("Sender: Send - SetFilePointer to message slot failed");
                }
                if (!WriteFile(h_file.get(), &msg_to_send, sizeof(Message), &bytes_op, NULL) || bytes_op != sizeof(Message)) {
                    ReleaseMutex(mutex.get()); ReleaseSemaphore(sem_empty.get(), 1, NULL);
                    throw WinApiException("Sender: Send - Failed to write message");
                }

                current_header.writeIndex = (current_header.writeIndex + 1) % current_header.maxMessages;
                current_header.messageCount++;

                if (SetFilePointer(h_file.get(), 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
                    ReleaseMutex(mutex.get()); // Состояние может быть несогласованным
                    throw WinApiException("Sender: Send - SetFilePointer to header for update failed");
                }
                if (!WriteFile(h_file.get(), &current_header, sizeof(FileHeader), &bytes_op, NULL) || bytes_op != sizeof(FileHeader)) {
                    ReleaseMutex(mutex.get());
                    throw WinApiException("Sender: Send - Failed to write updated header");
                }

                std::cout << "Sender: Message '" << message_text << "' sent." << std::endl;
                ReleaseMutex(mutex.get());
                if (!ReleaseSemaphore(sem_full.get(), 1, NULL)) {
                    throw WinApiException("Sender: Failed to release full semaphore");
                }

            }
            else if (command == "exit") {
                std::cout << "Sender: Exiting..." << std::endl;
                break;
            }
            else if (!command.empty()) { // Если была введена непустая неизвестная команда
                std::cout << "Sender: Unknown command '" << command << "'." << std::endl;
            }
            // Если строка была пустой, просто запросим ввод снова
        }

    }
    catch (const WinApiException& e) {
        std::cerr << "Sender Error (WinAPI): " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Sender Error: " << e.what() << std::endl;
        return 1;
    }
    std::cout << "Sender: Finished." << std::endl;
    return 0;
}