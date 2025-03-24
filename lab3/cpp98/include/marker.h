#ifndef MARKER_H
#define MARKER_H

#include "common.h"
#include <pthread.h>

struct ThreadData {
    int id;
    std::vector<int>* arr;
    bool exit;
    bool sleep;
};

struct GlobalData {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    std::vector<ThreadData*> threadData;
    int size;
};

// Глобальные данные для межпотокового взаимодействия
extern GlobalData g_data;

// Функция потока marker
void* marker_thread(void* arg);

// Инициализация глобальных данных
void init_global_data(int count_of_threads, int arr_size);

// Освобождение ресурсов
void cleanup_global_data();

// Проверка, все ли потоки спят
bool all_threads_sleep();

#endif // MARKER_H
