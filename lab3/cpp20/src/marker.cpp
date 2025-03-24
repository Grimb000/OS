#include "../include/marker.h"

MarkerManager::MarkerManager(int arr_size, int thread_count)
    : array(arr_size, 0),
      thread_exited(thread_count, false),
      thread_sleep(thread_count, false),
      thread_count(thread_count),
      exited_threads_count(0) {
}

MarkerManager::~MarkerManager() {
    // Остановить все потоки, если они еще работают
    for (size_t i = 0; i < threads.size(); ++i) {
        if (threads[i].joinable()) {
            std::unique_lock<std::mutex> lock(mtx);
            thread_exited[i] = true;
            lock.unlock();
            cv.notify_all();
            threads[i].join();
        }
    }
}

void MarkerManager::marker_function(size_t id) {
    srand(static_cast<unsigned>(id + 1));

    int size = array.size();
    int count_of_colored = 0;

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Проверка сигнала на завершение
        if (thread_exited[id]) {
            // Очистка помеченных элементов
            for (int i = 0; i < size; ++i) {
                if (array[i] == static_cast<int>(id) + 1) {
                    array[i] = 0;
                }
            }
            lock.unlock();
            cv.notify_all();
            break;
        }
        
        // Ожидание сигнала на продолжение работы
        cv.wait(lock, [this, id]() {
            return !thread_sleep[id] || thread_exited[id];
        });
        
        // Повторная проверка на сигнал завершения после пробуждения
        if (thread_exited[id]) {
            // Очистка помеченных элементов
            for (int i = 0; i < size; ++i) {
                if (array[i] == static_cast<int>(id) + 1) {
                    array[i] = 0;
                }
            }
            lock.unlock();
            cv.notify_all();
            break;
        }
        
        // Генерация случайного индекса
        int rng = rand() % size;
        
        if (array[rng] == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            array[rng] = id + 1;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            count_of_colored++;
            lock.unlock();
        } else {
            std::cout << "\nID потока: " << id + 1 << "\n"
                      << "Кол-во уникальных помеченных элементов: " << count_of_colored << "\n"
                      << "Невозможно пометить элемент под номером: " << rng << "\n\n";
            
            thread_sleep[id] = true;
            count_of_colored = 0;
            lock.unlock();
            cv.notify_all();
        }
    }
}

void MarkerManager::start_all_threads() {
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(&MarkerManager::marker_function, this, i);
    }
    cv.notify_all();
}

bool MarkerManager::all_threads_sleep() const {
    return std::all_of(thread_sleep.begin(), thread_sleep.end(), 
                      [this](bool sleep) { return sleep; });
}

bool MarkerManager::is_all_threads_sleep() const {
    std::unique_lock<std::mutex> lock(mtx);
    return all_threads_sleep();
}

void MarkerManager::terminate_thread(int id) {
    if (id < 1 || id > thread_count) {
        throw std::out_of_range("Недопустимый ID потока");
    }
    
    std::unique_lock<std::mutex> lock(mtx);
    thread_exited[id-1] = true;
    lock.unlock();
    cv.notify_all();
    
    if (threads[id-1].joinable()) {
        threads[id-1].join();
    }
    
    exited_threads_count++;
}

void MarkerManager::run_main_loop() {
    while (exited_threads_count < thread_count) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Ждем, пока все потоки не уснут
        cv.wait(lock, [this]() { 
            return all_threads_sleep() || exited_threads_count == thread_count - 1; 
        });
        
        // Если остался только один поток, завершаем цикл
        if (exited_threads_count == thread_count - 1) {
            lock.unlock();
            break;
        }
        
        // Вывод содержимого массива
        std::cout << "Текущее состояние массива:\n";
        for (int val : array) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
        
        lock.unlock();
        
        // Запрос ID потока для завершения
        int stop_id;
        std::cout << "\nВведите ID потока, который хотите остановить: ";
        while (true) {
            std::cin >> stop_id;
            if (stop_id <= 0 || stop_id > thread_count) {
                std::cout << "\nНекорректный ввод! Попробуйте ещё раз.\n"
                          << "Введите ID потока, который хотите остановить: ";
            } else {
                break;
            }
        }
        
        // Завершение выбранного потока
        terminate_thread(stop_id);
        
        // Вывод содержимого массива после завершения потока
        std::cout << "\nСостояние массива после завершения потока " << stop_id << ":\n";
        for (int val : array) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
        
        // Разбудить оставшиеся потоки
        std::unique_lock<std::mutex> wake_lock(mtx);
        for (int i = 0; i < thread_count; ++i) {
            if (!thread_exited[i]) {
                thread_sleep[i] = false;
            }
        }
        wake_lock.unlock();
        cv.notify_all();
    }
    
    // Дождаться завершения всех потоков
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
