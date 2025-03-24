#include "../include/marker.h"
#include <cassert>
#include <iostream>

// Тест создания менеджера
void test_manager_creation() {
    MarkerManager manager(10, 3);
    
    // Проверка, что массив инициализирован нулями
    for (int val : manager.get_array()) {
        assert(val == 0);
    }
    
    std::cout << "Тест создания менеджера успешно пройден." << std::endl;
}

// Тест функции определения спящих потоков (до запуска потоков)
void test_threads_sleep() {
    MarkerManager manager(10, 3);
    
    // До запуска потоков все флаги спящих потоков должны быть false
    assert(manager.is_all_threads_sleep() == false);
    
    std::cout << "Тест функции определения спящих потоков успешно пройден." << std::endl;
}

// Тест с запуском и остановкой одного потока
void test_thread_termination() {
    // Используем меньше значения для быстрого теста
    MarkerManager manager(5, 2);
    
    // Запуск потоков
    manager.start_all_threads();
    
    // Даем потокам немного поработать
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    try {
        // Остановка первого потока
        manager.terminate_thread(1);
        std::cout << "Тест остановки потока успешно пройден." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Ошибка при остановке потока: " << e.what() << std::endl;
        assert(false);
    }
}

int main() {
    test_manager_creation();
    test_threads_sleep();
    test_thread_termination();
    
    std::cout << "Все тесты для C++20 реализации пройдены успешно!" << std::endl;
    return 0;
}
