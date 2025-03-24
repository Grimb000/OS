#ifndef MARKER_H
#define MARKER_H

#include "common.h"

// Класс для управления потоками marker
class MarkerManager {
public:
    MarkerManager(int arr_size, int thread_count);
    ~MarkerManager();
    
    // Запустить все потоки marker
    void start_all_threads();
    
    // Запустить цикл обработки в main
    void run_main_loop();
    
    // Получить текущее состояние массива
    const std::vector<int>& get_array() const { return array; }
    
    // Для тестирования
    bool is_all_threads_sleep() const;
    void terminate_thread(int id);

private:
    // Функция потока marker
    void marker_function(size_t id);
    
    // Проверка, все ли потоки спят
    bool all_threads_sleep() const;
    
    std::vector<int> array;                   // Массив для маркировки
    std::vector<std::thread> threads;         // Потоки marker
    std::vector<bool> thread_exited;          // Флаги завершения потоков
    std::vector<bool> thread_sleep;           // Флаги сна потоков
    
    mutable std::mutex mtx;                   // Мьютекс для синхронизации
    std::condition_variable cv;               // Условная переменная
    
    int thread_count;                         // Количество потоков
    int exited_threads_count;                 // Количество завершенных потоков
};

#endif // MARKER_H
