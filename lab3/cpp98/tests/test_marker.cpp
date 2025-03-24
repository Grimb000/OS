#include "../include/marker.h"
#include <cassert>

// Тест на инициализацию глобальных данных
void test_init_global_data() {
    init_global_data(3, 10);
    
    assert(g_data.threadData.size() == 3);
    assert(g_data.size == 10);
    
    for (int i = 0; i < 3; ++i) {
        assert(g_data.threadData[i]->id == i);
        assert(g_data.threadData[i]->exit == false);
        assert(g_data.threadData[i]->sleep == false);
    }
    
    cleanup_global_data();
    assert(g_data.threadData.size() == 0);
    
    std::cout << "Тест инициализации глобальных данных пройден успешно." << std::endl;
}

// Тест функции all_threads_sleep
void test_all_threads_sleep() {
    init_global_data(3, 10);
    
    // В начале ни один поток не спит
    assert(all_threads_sleep() == false);
    
    // Усыпляем только первый поток
    g_data.threadData[0]->sleep = true;
    assert(all_threads_sleep() == false);
    
    // Усыпляем все потоки
    for (int i = 0; i < 3; ++i) {
        g_data.threadData[i]->sleep = true;
    }
    assert(all_threads_sleep() == true);
    
    // Завершаем один поток, остальные спят
    g_data.threadData[0]->exit = true;
    g_data.threadData[0]->sleep = false;
    assert(all_threads_sleep() == true);
    
    // Будим один из незавершенных потоков
    g_data.threadData[1]->sleep = false;
    assert(all_threads_sleep() == false);
    
    cleanup_global_data();
    
    std::cout << "Тест функции all_threads_sleep пройден успешно." << std::endl;
}

int main() {
    test_init_global_data();
    test_all_threads_sleep();
    
    std::cout << "Все тесты для C++98 реализации пройдены успешно!" << std::endl;
    return 0;
}
