# Исполняемый файл для Receiver (модернизированный)
add_executable(Receiver Receiver.cpp common.h utils.h)

# Исполняемый файл для Sender (модернизированный)
add_executable(Sender Sender.cpp common.h utils.h)

# Указываем, где CMake должен размещать исполняемые файлы
set_target_properties(Receiver Sender PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
