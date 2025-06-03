#include <iostream>
#include <string>
#include <vector>
#include <sstream>     // Для std::ostringstream (или std::format в C++20)
#include <algorithm>   // Для std::all_of (если понадобится)
#include <stdexcept>   // Для std::invalid_argument

#include "common.h"
#include "utils.h" // Для unique_handle и WinApiException

void initialize_binary_file(const std::string& file_name_str, int num_records) {
    unique_handle h_file(CreateFileA(
        file_name_str.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    ));

    if (h_file.get() == INVALID_HANDLE_VALUE) {
        throw WinApiException("Receiver: Failed to create binary file '" + file_name_str + "'");
    }

    FileHeader header = {}; // Value initialization
    header.writeIndex = 0;
    header.readIndex = 0;
    header.maxMessages = static_cast<UINT>(num_records);
    header.messageCount = 0;

    DWORD bytes_written;
    if (!WriteFile(h_file.get(), &header, sizeof(FileHeader), &bytes_written, NULL) || bytes_written != sizeof(FileHeader)) {
        throw WinApiException("Receiver: Failed to write file header");
    }

    Message empty_msg = {}; // data будет zero-initialized
    for (int i = 0; i < num_records; ++i) {
        if (!WriteFile(h_file.get(), &empty_msg, sizeof(Message), &bytes_written, NULL) || bytes_written != sizeof(Message)) {
            throw WinApiException("Receiver: Failed to initialize message slot in file");
        }
    }
    std::cout << "Receiver: Binary file '" << file_name_str << "' created with " << num_records << " records capacity." << std::endl;
}

int main() {
    try {
        std::string file_name;
        int num_records;
        int num_senders;

        std::cout << "Receiver: Enter binary file name: ";
        std::cin >> file_name;
        std::cout << "Receiver: Enter number of records (queue capacity): ";
        std::cin >> num_records;

        if (num_records <= 0) {
            throw std::invalid_argument("Number of records must be positive.");
        }

        initialize_binary_file(file_name, num_records);

        // Создание объектов синхронизации
        unique_handle mutex(CreateMutexA(NULL, FALSE, SHARED_FILE_MUTEX_NAME_MODERN.c_str()));
        if (!mutex) throw WinApiException("Receiver: Failed to create mutex");

        unique_handle sem_empty(CreateSemaphoreA(NULL, num_records, num_records, SEMAPHORE_EMPTY_NAME_MODERN.c_str()));
        if (!sem_empty) throw WinApiException("Receiver: Failed to create empty semaphore");

        unique_handle sem_full(CreateSemaphoreA(NULL, 0, num_records, SEMAPHORE_FULL_NAME_MODERN.c_str()));
        if (!sem_full) throw WinApiException("Receiver: Failed to create full semaphore");

        std::cout << "Receiver: Enter number of Sender processes (max " << MAX_SENDERS_ALLOWED << "): ";
        std::cin >> num_senders;

        if (num_senders <= 0 || num_senders > MAX_SENDERS_ALLOWED) {
            throw std::invalid_argument("Invalid number of Senders.");
        }

        std::vector<unique_handle> sender_process_handles;
        std::vector<unique_handle> sender_thread_handles; // Store thread handles from CreateProcess
        std::vector<HANDLE> sender_ready_event_raw_handles(num_senders); // Raw handles for WaitForMultipleObjects
        std::vector<unique_handle> sender_ready_event_unique_handles; // To manage lifetime


        for (int i = 0; i < num_senders; ++i) {
            std::ostringstream event_name_stream;
            event_name_stream << SENDER_READY_EVENT_PREFIX_MODERN << i;
            std::string sender_ready_event_name = event_name_stream.str();

            HANDLE raw_event_handle = CreateEventA(NULL, TRUE, FALSE, sender_ready_event_name.c_str()); // TRUE - manual reset
            if (raw_event_handle == NULL) {
                throw WinApiException("Receiver: Failed to create sender ready event for Sender " + std::to_string(i));
            }
            sender_ready_event_unique_handles.emplace_back(raw_event_handle);
            sender_ready_event_raw_handles[i] = raw_event_handle;


            // Имя исполняемого файла Sender'а (предполагается, что он в той же директории или в PATH)
            // Для большей надежности можно было бы передавать полный путь к SenderModern.exe
            std::string sender_exe_name = "Sender.exe"; // или SenderModern (без .exe для CreateProcess)
            std::ostringstream command_line_stream;
            command_line_stream << sender_exe_name << " " << file_name << " " << sender_ready_event_name;
            std::string command_line = command_line_stream.str();

            std::cout << "Receiver: Launching Sender " << i << " with command: " << command_line << std::endl;

            auto [proc_handle, thread_handle] = create_process_cpp("", command_line); // Первый аргумент пуст, если имя модуля в командной строке

            sender_process_handles.push_back(std::move(proc_handle));
            sender_thread_handles.push_back(std::move(thread_handle)); // Закроем хендл потока, т.к. он не нужен далее

            std::cout << "Receiver: Sender process " << i << " launched." << std::endl;
        }

        if (num_senders > 0) {
            std::cout << "Receiver: Waiting for all Senders to be ready..." << std::endl;
            DWORD wait_result = WaitForMultipleObjects(num_senders, sender_ready_event_raw_handles.data(), TRUE, INFINITE);
            if (wait_result >= WAIT_OBJECT_0 && wait_result < WAIT_OBJECT_0 + num_senders) {
                std::cout << "Receiver: All Senders are ready." << std::endl;
            }
            else if (wait_result == WAIT_FAILED) {
                throw WinApiException("Receiver: Failed waiting for sender ready events.");
            }
            else {
                throw std::runtime_error("Receiver: Unexpected result from WaitForMultipleObjects: " + std::to_string(wait_result));
            }
        }
        // unique_handles для событий закроются автоматически при выходе из области видимости sender_ready_event_unique_handles

        // Главный цикл Receiver'а
        unique_handle h_file_main(CreateFileA( // Открываем файл для операций чтения/записи
            file_name.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
        if (h_file_main.get() == INVALID_HANDLE_VALUE) {
            throw WinApiException("Receiver: Failed to open binary file for operations");
        }

        std::string command;
        while (true) {
            std::cout << "\nReceiver: Enter command ('read' or 'exit'): ";
            std::cin >> command;

            if (command == "read") {
                std::cout << "Receiver: Waiting for a message..." << std::endl;
                if (WaitForSingleObject(sem_full.get(), INFINITE) != WAIT_OBJECT_0) {
                    throw WinApiException("Receiver: Failed to wait for full semaphore");
                }

                if (WaitForSingleObject(mutex.get(), INFINITE) != WAIT_OBJECT_0) {
                    ReleaseSemaphore(sem_full.get(), 1, NULL); // Откатить sem_full
                    throw WinApiException("Receiver: Failed to wait for mutex");
                }

                FileHeader current_header;
                DWORD bytes_op;

                if (SetFilePointer(h_file_main.get(), 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
                    ReleaseMutex(mutex.get()); ReleaseSemaphore(sem_full.get(), 1, NULL);
                    throw WinApiException("Receiver: Read - SetFilePointer to header failed");
                }
                if (!ReadFile(h_file_main.get(), &current_header, sizeof(FileHeader), &bytes_op, NULL) || bytes_op != sizeof(FileHeader)) {
                    ReleaseMutex(mutex.get()); ReleaseSemaphore(sem_full.get(), 1, NULL);
                    throw WinApiException("Receiver: Read - Failed to read header");
                }

                // Семафор Full гарантирует, что current_header.messageCount > 0
                Message received_msg;
                long message_offset = sizeof(FileHeader) + current_header.readIndex * sizeof(Message);
                if (SetFilePointer(h_file_main.get(), message_offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
                    ReleaseMutex(mutex.get()); ReleaseSemaphore(sem_full.get(), 1, NULL);
                    throw WinApiException("Receiver: Read - SetFilePointer to message failed");
                }
                if (!ReadFile(h_file_main.get(), &received_msg, sizeof(Message), &bytes_op, NULL) || bytes_op != sizeof(Message)) {
                    ReleaseMutex(mutex.get()); ReleaseSemaphore(sem_full.get(), 1, NULL);
                    // Важно: сообщение не прочитано, но семафор full был взят. Его нужно "вернуть".
                    // Однако, если файл поврежден, это может привести к проблемам.
                    // Лучше завершиться с ошибкой.
                    throw WinApiException("Receiver: Read - Failed to read message");
                }

                // Вывод сообщения (безопасный, т.к. data - массив char)
                // Для вывода как C-строки, убедимся, что она null-терминирована в пределах MAX_MESSAGE_LENGTH
                std::string msg_str(received_msg.data, strnlen(received_msg.data, MAX_MESSAGE_LENGTH));
                std::cout << "Receiver: Received message: " << msg_str << std::endl;

                current_header.readIndex = (current_header.readIndex + 1) % current_header.maxMessages;
                current_header.messageCount--;

                if (SetFilePointer(h_file_main.get(), 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
                    // Критическая ошибка, состояние файла может быть несогласованным
                    ReleaseMutex(mutex.get());
                    throw WinApiException("Receiver: Read - SetFilePointer to header for update failed");
                }
                if (!WriteFile(h_file_main.get(), &current_header, sizeof(FileHeader), &bytes_op, NULL) || bytes_op != sizeof(FileHeader)) {
                    ReleaseMutex(mutex.get());
                    throw WinApiException("Receiver: Read - Failed to write updated header");
                }

                ReleaseMutex(mutex.get());
                if (!ReleaseSemaphore(sem_empty.get(), 1, NULL)) {
                    throw WinApiException("Receiver: Failed to release empty semaphore");
                }

            }
            else if (command == "exit") {
                std::cout << "Receiver: Exiting..." << std::endl;
                break;
            }
            else {
                std::cout << "Receiver: Unknown command." << std::endl;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Очистка буфера ввода
            }
        }

    }
    catch (const WinApiException& e) {
        std::cerr << "Receiver Error (WinAPI): " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Receiver Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Receiver: Shutting down Senders and cleaning up..." << std::endl;
    // unique_handle для процессов и событий закроются автоматически.
    // Если нужно принудительно завершить Sender'ы раньше:
    // for (auto& proc_handle : sender_process_handles) {
    //     if (proc_handle.get()) TerminateProcess(proc_handle.get(), 0);
    // }
    std::cout << "Receiver: Finished." << std::endl;
    return 0;
}