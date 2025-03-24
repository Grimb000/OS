#include "../include/marker.h"

GlobalData g_data;

void* marker_thread(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    
    int id = data->id;
    std::vector<int>* arr = data->arr;
    int size = arr->size();
    int count_of_colored = 0;
    
    srand(id + 1); // Инициализация генератора случайных чисел
    
    while (true) {
        pthread_mutex_lock(&g_data.mutex);
        
        // Проверка на сигнал завершения
        if (data->exit) {
            // Очистка помеченных элементов
            for (int i = 0; i < size; ++i) {
                if ((*arr)[i] == id + 1) {
                    (*arr)[i] = 0;
                }
            }
            pthread_mutex_unlock(&g_data.mutex);
            pthread_cond_broadcast(&g_data.condition);
            break;
        }
        
        // Ожидание сигнала продолжения работы
        while (data->sleep && !data->exit) {
            pthread_cond_wait(&g_data.condition, &g_data.mutex);
        }
        
        // Повторная проверка на сигнал завершения после пробуждения
        if (data->exit) {
            // Очистка помеченных элементов
            for (int i = 0; i < size; ++i) {
                if ((*arr)[i] == id + 1) {
                    (*arr)[i] = 0;
                }
            }
            pthread_mutex_unlock(&g_data.mutex);
            pthread_cond_broadcast(&g_data.condition);
            break;
        }
        
        // Генерация случайного индекса
        int rng = rand() % size;
        
        if ((*arr)[rng] == 0) {
            sleep_ms(5);
            (*arr)[rng] = id + 1;
            sleep_ms(5);
            count_of_colored++;
            pthread_mutex_unlock(&g_data.mutex);
        } else {
            std::cout << "\nID потока: " << id + 1 << "\n"
                      << "Кол-во уникальных помеченных элементов: " << count_of_colored << "\n"
                      << "Невозможно пометить элемент под номером: " << rng << "\n\n";
            
            data->sleep = true;
            count_of_colored = 0;
            pthread_mutex_unlock(&g_data.mutex);
            pthread_cond_broadcast(&g_data.condition);
        }
    }
    
    return NULL;
}

void init_global_data(int count_of_threads, int arr_size) {
    pthread_mutex_init(&g_data.mutex, NULL);
    pthread_cond_init(&g_data.condition, NULL);
    g_data.size = arr_size;
    
    for (int i = 0; i < count_of_threads; ++i) {
        ThreadData* data = new ThreadData();
        data->id = i;
        data->exit = false;
        data->sleep = false;
        g_data.threadData.push_back(data);
    }
}

void cleanup_global_data() {
    pthread_mutex_destroy(&g_data.mutex);
    pthread_cond_destroy(&g_data.condition);
    
    for (size_t i = 0; i < g_data.threadData.size(); ++i) {
        delete g_data.threadData[i];
    }
    g_data.threadData.clear();
}

bool all_threads_sleep() {
    for (size_t i = 0; i < g_data.threadData.size(); ++i) {
        if (!g_data.threadData[i]->exit && !g_data.threadData[i]->sleep) {
            return false;
        }
    }
    return true;
}
