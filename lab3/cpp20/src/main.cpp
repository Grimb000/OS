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
    
    MarkerManager manager(size, count_of_threads);
    
    // Запуск потоков marker
    manager.start_all_threads();
    
    // Запуск основного цикла
    manager.run_main_loop();
    
    // Вывод финального состояния массива
    std::cout << "\nКонечный массив:\n";
    for (int val : manager.get_array()) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
