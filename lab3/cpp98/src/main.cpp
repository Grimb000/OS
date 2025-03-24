#include "../include/marker.h"

int main() {
    #ifdef _WIN32
    setlocale(LC_ALL, "ru");
    #endif
    
    int size;
    std::cout << "Введите размер массива: ";
    while (true) {
        std::cin >> size;
        
        if (size <= 0) {
            std::cout << "Некорректный ввод! Попробуйте ещё раз.\n"
                      << "Введите размер массива: ";
            continue;
        }
        break;
    }
    
    // Инициализация массива нулями
    std::vector<int> arr(size, 0);
    
    int count_of_threads;
    std::cout << "Введите количество потоков marker: ";
    while (true) {
        std::cin >> count_of_threads;
        
        if (count_of_threads <= 0) {
            std::cout << "Некорректный ввод! Попробуйте ещё раз.\n"
                      << "Введите количество потоков marker: ";
            continue;
        }
        break;
    }
    
    // Инициализация глобальных данных
    init_global_data(count_of_threads, size);
    
    // Создание потоков
    std::vector<pthread_t> threads(count_of_threads);
    for (int i = 0; i < count_of_threads; ++i) {
        g_data.threadData[i]->arr = &arr;
        pthread_create(&threads[i], NULL, marker_thread, g_data.threadData[i]);
    }
    
    // Сигнал всем потокам начать работу
    pthread_cond_broadcast(&g_data.condition);
    
    int exited_threads_count = 0;
    while (exited_threads_count < count_of_threads) {
        pthread_mutex_lock(&g_data.mutex);
        
        // Ждем, пока все потоки не уснут
        while (!all_threads_sleep() && exited_threads_count < count_of_threads) {
            pthread_cond_wait(&g_data.condition, &g_data.mutex);
        }
        
        // Если все потоки, кроме одного, завершились
        if (exited_threads_count == count_of_threads - 1) {
            pthread_mutex_unlock(&g_data.mutex);
            break;
        }
        
        // Вывод содержимого массива
        std::cout << "Текущее состояние массива:\n";
        for (int i = 0; i < size; ++i) {
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;
        
        // Запрос ID потока для завершения
        int stop_id;
        std::cout << "\nВведите ID потока, который хотите остановить: ";
        while (true) {
            std::cin >> stop_id;
            if (stop_id <= 0 || stop_id > count_of_threads) {
                std::cout << "\nНекорректный ввод! Попробуйте ещё раз.\n"
                          << "Введите ID потока, который хотите остановить: ";
            } else {
                break;
            }
        }
        
        // Помечаем поток для завершения
        g_data.threadData[stop_id-1]->exit = true;
        pthread_mutex_unlock(&g_data.mutex);
        
        // Отправляем сигнал всем потокам
        pthread_cond_broadcast(&g_data.condition);
        
        // Ждем завершения потока
        pthread_join(threads[stop_id-1], NULL);
        
        exited_threads_count++;
        
        // Вывод содержимого массива после завершения потока
        std::cout << "\nСостояние массива после завершения потока " << stop_id << ":\n";
        for (int i = 0; i < size; ++i) {
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;
        
        // Разбудить оставшиеся потоки
        pthread_mutex_lock(&g_data.mutex);
        for (int i = 0; i < count_of_threads; ++i) {
            if (!g_data.threadData[i]->exit) {
                g_data.threadData[i]->sleep = false;
            }
        }
        pthread_mutex_unlock(&g_data.mutex);
        pthread_cond_broadcast(&g_data.condition);
    }
    
    // Ждем завершения всех потоков
    for (int i = 0; i < count_of_threads; ++i) {
        if (!g_data.threadData[i]->exit) {
            pthread_join(threads[i], NULL);
        }
    }
    
    // Вывод конечного состояния массива
    std::cout << "\nКонечный массив:\n";
    for (int i = 0; i < size; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
    
    // Освобождение ресурсов
    cleanup_global_data();
    
    return 0;
}
