# Исполняемый файл для Receiver
add_executable(Receiver Receiver.cpp common.h)

# Исполняемый файл для Sender
add_executable(Sender Sender.cpp common.h)

# Указываем, где CMake должен размещать исполняемые файлы (например, в корне билд-директории)
set_target_properties(Receiver Sender PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
