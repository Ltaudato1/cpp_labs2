#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <atomic>
#include <stdexcept>
#include <list>
#include <condition_variable>

template<typename T>
class LifeCycleContainer {
private:
    std::unordered_map<T*, std::chrono::steady_clock::time_point> creation_times;
    
public:
    void on_element_created(T* element) {
        creation_times[element] = std::chrono::steady_clock::now();
    }
    
    void on_element_destroyed(T* element) {
        auto it = creation_times.find(element);
        if (it != creation_times.end()) {
            auto lifespan = std::chrono::steady_clock::now() - it->second;
            creation_times.erase(it);
        }
    }
};

struct AccessKeyInfo {
    std::chrono::steady_clock::time_point last_access;
    int remaining_usage;
    bool first_operation;
    
    AccessKeyInfo(int usage_limit) 
        : last_access(std::chrono::steady_clock::now())
        , remaining_usage(usage_limit)
        , first_operation(true) {}
};

template<typename T>
struct QueueElement {
    T* element;
    std::chrono::steady_clock::time_point creation_time;
    std::chrono::steady_clock::duration lifetime;
    
    QueueElement(T* elem, std::chrono::steady_clock::duration lifetime_duration) 
        : element(elem)
        , creation_time(std::chrono::steady_clock::now())
        , lifetime(lifetime_duration) {}
    
    bool is_expired() const {
        auto now = std::chrono::steady_clock::now();
        return (now - creation_time) > lifetime;
    }
};

template<typename T>
class Queue {
private:
    std::list<QueueElement<T>> queue_elements;
    std::mutex queue_mutex;
    std::condition_variable cleanup_cv;
    std::thread cleanup_thread;
    std::atomic<bool> stop_cleanup{false};
    
    int capacity;
    std::chrono::milliseconds delta_time;
    int usage_limit_per_key;
    std::chrono::milliseconds element_lifetime;
    
    std::unordered_map<int, AccessKeyInfo> access_keys;
    std::mutex keys_mutex;
    std::atomic<int> next_key{0};
    
    LifeCycleContainer<T>* lifecycle_observer;
    
    void cleanup_loop() {
        while (!stop_cleanup) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
            if (cleanup_cv.wait_for(lock, std::chrono::milliseconds(100), 
                                   [this]() { return stop_cleanup.load(); })) {
                break;
            }
            
            auto it = queue_elements.begin();
            while (it != queue_elements.end()) {
                if (it->is_expired()) {
                    if (lifecycle_observer) {
                        lifecycle_observer->on_element_destroyed(it->element);
                    }
                    delete it->element;
                    it = queue_elements.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }
    
    void cleanup_expired_elements() {
        auto it = queue_elements.begin();
        while (it != queue_elements.end()) {
            if (it->is_expired()) {
                if (lifecycle_observer) {
                    lifecycle_observer->on_element_destroyed(it->element);
                }
                delete it->element;
                it = queue_elements.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    bool can_access_key(int key) {
        std::lock_guard<std::mutex> lock(keys_mutex);
        auto it = access_keys.find(key);
        if (it == access_keys.end()) {
            return false;
        }
        
        auto& info = it->second;
        
        if (info.remaining_usage <= 0) {
            access_keys.erase(it);
            return false;
        }
        
        if (info.first_operation) {
            info.first_operation = false;
            info.last_access = std::chrono::steady_clock::now();
            info.remaining_usage--;
            return true;
        }
        
        auto now = std::chrono::steady_clock::now();
        auto time_since_last_access = now - info.last_access;
        
        if (time_since_last_access < delta_time) {
            return false;
        }
        
        info.last_access = now;
        info.remaining_usage--;
        return true;
    }
    
public:
    Queue(int capacity, 
          std::chrono::milliseconds delta_time,
          int usage_limit_per_key,
          std::chrono::milliseconds element_lifetime = std::chrono::seconds(30),  // По умолчанию 30 секунд
          LifeCycleContainer<T>* observer = nullptr)
        : capacity(capacity)
        , delta_time(delta_time)
        , usage_limit_per_key(usage_limit_per_key)
        , element_lifetime(element_lifetime)
        , lifecycle_observer(observer) {
        
        if (capacity <= 0) {
            throw std::invalid_argument("Capacity must be positive");
        }
        if (delta_time.count() < 0) {
            throw std::invalid_argument("Delta time must be non-negative");
        }
        if (usage_limit_per_key <= 0) {
            throw std::invalid_argument("Usage limit must be positive");
        }
        if (element_lifetime.count() <= 0) {
            throw std::invalid_argument("Element lifetime must be positive");
        }
        
        // Запускаем поток очистки
        cleanup_thread = std::thread(&Queue::cleanup_loop, this);
    }
    
    ~Queue() {
        stop_cleanup = true;
        cleanup_cv.notify_all();
        if (cleanup_thread.joinable()) {
            cleanup_thread.join();
        }
        
        std::lock_guard<std::mutex> lock(queue_mutex);
        for (auto& elem : queue_elements) {
            if (lifecycle_observer) {
                lifecycle_observer->on_element_destroyed(elem.element);
            }
            delete elem.element;
        }
        queue_elements.clear();
    }
    
    int get_access_key() {
        int key = next_key++;
        std::lock_guard<std::mutex> lock(keys_mutex);
        access_keys.emplace(key, AccessKeyInfo(usage_limit_per_key));
        return key;
    }
    
    void push(int key, T value) {
        if (!can_access_key(key)) {
            throw std::runtime_error("Access denied: invalid key, usage limit exceeded, or delta time not passed");
        }
        
        std::lock_guard<std::mutex> lock(queue_mutex);
        
        if (queue_elements.size() >= capacity) {
            throw std::runtime_error("Queue is full");
        }
        
        T* element = new T(std::move(value));
        queue_elements.emplace_back(element, element_lifetime);
        
        if (lifecycle_observer) {
            lifecycle_observer->on_element_created(element);
        }
    }
    
    T pop(int key) {
        if (!can_access_key(key)) {
            throw std::runtime_error("Access denied: invalid key, usage limit exceeded, or delta time not passed");
        }
        
        std::lock_guard<std::mutex> lock(queue_mutex);
        
        cleanup_expired_elements();
        
        if (queue_elements.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        
        auto& front_elem = queue_elements.front();
        
        if (front_elem.is_expired()) {
            if (lifecycle_observer) {
                lifecycle_observer->on_element_destroyed(front_elem.element);
            }
            delete front_elem.element;
            queue_elements.pop_front();
            throw std::runtime_error("Element expired");
        }
        
        T* element = front_elem.element;
        T value = std::move(*element);
        
        if (lifecycle_observer) {
            lifecycle_observer->on_element_destroyed(element);
        }
        
        delete element;
        queue_elements.pop_front();
        return value;
    }
    
    size_t size() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        cleanup_expired_elements();
        return queue_elements.size();
    }
    
    bool empty() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        cleanup_expired_elements();
        return queue_elements.empty();
    }
    
    void cleanup() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        cleanup_expired_elements();
    }
};

// ==================== ЮНИТ ТЕСТЫ ====================

void test_queue_creation() {
    std::cout << "Test 1: Queue creation... ";
    try {
        LifeCycleContainer<int> observer;
        Queue<int> queue(10, std::chrono::milliseconds(100), 5, std::chrono::seconds(1), &observer);
        std::cout << "PASSED" << std::endl;
    } catch (std::exception const& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
    }
}

void test_push_pop_operations() {
    std::cout << "Test 2: Push and pop operations... ";
    try {
        LifeCycleContainer<int> observer;
        Queue<int> queue(5, std::chrono::milliseconds(50), 10, std::chrono::seconds(10), &observer);
        
        int key = queue.get_access_key();
        queue.push(key, 42);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        queue.push(key, 100);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        int val1 = queue.pop(key);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        int val2 = queue.pop(key);
        
        if (val1 == 42 && val2 == 100) {
            std::cout << "PASSED" << std::endl;
        } else {
            std::cout << "FAILED: Wrong values popped" << std::endl;
        }
    } catch (std::exception const& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
    }
}

void test_delta_time_constraint() {
    std::cout << "Test 3: Delta time constraint... ";
    try {
        LifeCycleContainer<int> observer;
        Queue<int> queue(5, std::chrono::milliseconds(100), 10, std::chrono::seconds(10), &observer);
        
        int key = queue.get_access_key();
        queue.push(key, 1);
        
        try {
            queue.push(key, 2);
            std::cout << "FAILED: Should have thrown exception" << std::endl;
        } catch (std::runtime_error const& e) {
            std::cout << "PASSED" << std::endl;
        }
    } catch (std::exception const& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
    }
}

void test_usage_limit() {
    std::cout << "Test 4: Usage limit per key... ";
    try {
        LifeCycleContainer<int> observer;
        Queue<int> queue(10, std::chrono::milliseconds(10), 2, std::chrono::seconds(10), &observer);
        
        int key = queue.get_access_key();
        
        queue.push(key, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
        queue.push(key, 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
        try {
            queue.push(key, 3);
            std::cout << "FAILED: Should have thrown exception after usage limit" << std::endl;
        } catch (std::runtime_error const& e) {
            std::cout << "PASSED" << std::endl;
        }
    } catch (std::exception const& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
    }
}

void test_multiple_keys_independence() {
    std::cout << "Test 5: Multiple keys independence... ";
    try {
        LifeCycleContainer<int> observer;
        Queue<int> queue(10, std::chrono::milliseconds(100), 3, std::chrono::seconds(10), &observer);
        
        int key1 = queue.get_access_key();
        int key2 = queue.get_access_key();
        
        queue.push(key1, 1);
        
        try {
            queue.push(key2, 2);
            std::cout << "PASSED" << std::endl;
        } catch (std::runtime_error const& e) {
            std::cout << "FAILED: " << e.what() << std::endl;
        }
    } catch (std::exception const& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
    }
}

void test_element_expiration() {
    std::cout << "Test 6: Element expiration... ";
    try {
        LifeCycleContainer<int> observer;
        Queue<int> queue(10, std::chrono::milliseconds(10), 10, std::chrono::milliseconds(50), &observer);
        
        int key = queue.get_access_key();
        
        queue.push(key, 42);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        
        try {
            queue.pop(key);
            std::cout << "FAILED: Should have thrown exception for expired element" << std::endl;
        } catch (std::runtime_error const& e) {
                std::cout << "PASSED" << std::endl;
        }
    } catch (std::exception const& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
    }
}

void test_automatic_cleanup() {
    std::cout << "Test 7: Automatic cleanup... ";
    try {
        LifeCycleContainer<int> observer;
        Queue<int> queue(10, std::chrono::milliseconds(10), 10, std::chrono::milliseconds(100), &observer);
        
        int key = queue.get_access_key();
        
        queue.push(key, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        queue.push(key, 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        queue.push(key, 3);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        if (queue.size() == 2) {
            int val = queue.pop(key);
            if (val == 2) {
                std::cout << "PASSED" << std::endl;
            } else {
                std::cout << "FAILED: Wrong value after cleanup" << std::endl;
            }
        } else {
            std::cout << "FAILED: Wrong size after cleanup: " << queue.size() << std::endl;
        }
    } catch (std::exception const& e) {
        std::cout << "FAILED: " << e.what() << std::endl;
    }
}

void run_all_tests() {
    std::cout << "Running all tests..." << std::endl;
    std::cout << "==================================" << std::endl;
    
    test_queue_creation();
    test_push_pop_operations();
    test_delta_time_constraint();
    test_usage_limit();
    test_multiple_keys_independence();
    test_element_expiration();
    test_automatic_cleanup();
    
    std::cout << "==================================" << std::endl;
    std::cout << "All tests completed!" << std::endl;
}

int main() {
    run_all_tests();
    return 0;
}