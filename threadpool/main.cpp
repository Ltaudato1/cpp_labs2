#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <future>
#include <chrono>
#include <algorithm>
#include <random>
#include <cmath>
#include <execution>

class ThreadPool {
public:
    ThreadPool(size_t num_threads) 
        : stop(false), 
          active_tasks(0) {
        workers.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });
                        
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                        active_tasks++;
                    }
                    
                    task();
                    
                    {
                        std::lock_guard<std::mutex> lock(queue_mutex);
                        active_tasks--;
                        if (active_tasks == 0 && tasks.empty()) {
                            completion_condition.notify_all();
                        }
                    }
                }
            });
        }
    }

    template<class F>
    void execute_no_future(F&& f) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    template<class F>
    auto execute(F&& f) -> std::future<decltype(f())> {
        using return_type = decltype(f());
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::forward<F>(f)
        );
        
        std::future<return_type> res = task->get_future();
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    void wait_all() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        completion_condition.wait(lock, [this] {
            return tasks.empty() && active_tasks == 0;
        });
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable completion_condition;
    std::atomic<bool> stop;
    std::atomic<int> active_tasks;
};

// =================== Parallel Sort ===================

void parallel_sort(std::vector<double>& arr, size_t num_threads) {

    ThreadPool pool(num_threads);
    std::vector<std::future<void>> futures;
    
    size_t chunk_size = arr.size() / num_threads;
    
    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == num_threads - 1) ? arr.size() : start + chunk_size;
        
        futures.push_back(pool.execute([&arr, start, end]() {
            std::sort(arr.begin() + start, arr.begin() + end);
        }));
    }
    
    for (auto& future : futures) {
        future.get();
    }
    
    std::vector<double> temp(arr.size());
    size_t merge_chunk_size = chunk_size;
    
    while (merge_chunk_size < arr.size()) {
        std::vector<std::future<void>> merge_futures;
        
        for (size_t i = 0; i < arr.size(); i += 2 * merge_chunk_size) {
            size_t left = i;
            size_t mid = std::min(i + merge_chunk_size, arr.size());
            size_t right = std::min(i + 2 * merge_chunk_size, arr.size());
            
            merge_futures.push_back(pool.execute([&arr, &temp, left, mid, right]() {
                std::merge(
                    arr.begin() + left,
                    arr.begin() + mid,
                    arr.begin() + mid,
                    arr.begin() + right,
                    temp.begin() + left
                );
            }));
        }
        
        for (auto& future : merge_futures) {
            future.get();
        }
        
        std::swap(arr, temp);
        merge_chunk_size *= 2;
    }
}

// =================== Тесты производительности ===================

void benchmark_sort() {
    std::cout << "\n=== Бенчмарк сортировки ===\n";
    
    const int SIZES[] = {1000, 10000, 100000, 1000000, 5000000};
    const int THREADS[] = {1, 2, 4, 8, 12};
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int size : SIZES) {
        std::cout << "\nРазмер массива: " << size << std::endl;
        std::cout << "-------------------\n";
        
        std::vector<double> base_arr(size);
        std::uniform_real_distribution<double> dis(1.0, size * 10.0);
        
        for (int i = 0; i < size; ++i) {
            base_arr[i] = dis(gen);
        }
        
        for (int threads : THREADS) {
            if (threads > std::thread::hardware_concurrency() && threads != 1) {
                continue;
            }
            
            auto arr = base_arr;
            auto start = std::chrono::high_resolution_clock::now();
            
            if (threads == 1) {
                std::sort(arr.begin(), arr.end());
            } else {
                parallel_sort(arr, threads);
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            bool sorted = std::is_sorted(arr.begin(), arr.end());
            
            std::cout << "Потоков: " << threads 
                     << ", Время: " << duration.count() / 1000.0 << " мс"
                     << ", Корректно: " << (sorted ? "да" : "нет") << std::endl;
        }

        auto arr = base_arr;
        auto start = std::chrono::high_resolution_clock::now();
        std::sort(std::execution::par, arr.begin(), arr.end());
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        bool sorted = std::is_sorted(arr.begin(), arr.end());
        std::cout << "С std::execution::par: "  
                    << ", Время: " << duration.count() / 1000.0 << " мс"
                   << ", Корректно: " << (sorted ? "да" : "нет") << std::endl;
    }
}

void run_all_tests() {
    benchmark_sort();
}

int main() {
    benchmark_sort();
    return 0;
}