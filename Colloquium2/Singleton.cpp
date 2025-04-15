#include <iostream>
#include <mutex>
#include <atomic>
#include <memory>

//Стратегии синхронизации
class NoLock {
public:
    void Lock() {}
    void Unlock() {}
};

class MutexLock {
    std::mutex mtx;
public:
    void Lock() { mtx.lock(); }
    void Unlock() { mtx.unlock(); }
};

//Стратегии создания
template <typename T>
class CreateStatic {
protected:
    static T* Create() {
        static T instance;
        return &instance;
    }
};

template <typename T>
class CreateNew {
protected:
    static T* Create() {
        return new T();
    }
};

//Стратегии времени жизни
template <typename T>
class DefaultLifetime {
public:
    static void Destroy(T*) {}
};

template <typename T>
class AtExitLifetime {
public:
    static void Destroy(T* obj) {
        delete obj;
    }
};

//Singleton
template <
    typename T,
    template <typename> class CreationPolicy = CreateStatic,
    template <typename> class LifetimePolicy = DefaultLifetime,
    template <typename> class ThreadingPolicy = NoLock
>
class Singleton {
public:
    static T& Instance() {
        static std::atomic<T*> instance = nullptr;
        ThreadingPolicy<T> lock;

        if (!instance.load(std::memory_order_acquire)) {
            lock.Lock();
            if (!instance.load(std::memory_order_relaxed)) {
                instance.store(CreationPolicy<T>::Create(), std::memory_order_release);
                LifetimePolicy<T>::ScheduleDestruction(&DestroySingleton);
            }
            lock.Unlock();
        }
        return *instance;
    }

protected:
    static void DestroySingleton() {
        ThreadingPolicy<T> lock;
        lock.Lock();
        T* obj = instance.load(std::memory_order_relaxed);
        instance.store(nullptr, std::memory_order_release);
        LifetimePolicy<T>::Destroy(obj);
        lock.Unlock();
    }

    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

//Пример использования
class Logger {
public:
    void Log(const std::string& msg) {
        std::cout << "[LOG] " << msg << std::endl;
    }
};

int main() {
    // Singleton с Mutex-синхронизацией и автоматическим удалением
    using SafeLogger = Singleton<Logger, CreateNew, AtExitLifetime, MutexLock>;

    SafeLogger::Instance().Log("Hello, Singleton!");
    SafeLogger::Instance().Log("This is thread-safe!");

    return 0;
}
