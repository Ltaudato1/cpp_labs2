#include <iostream>
#include <list>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <cassert>
#include <thread>
#include <vector>
#include <chrono>

template<typename K, typename V>
class LRUCache {
public:
    explicit LRUCache(size_t capacity)
        : capacity(capacity) {
        if (capacity <= 0) {
            throw std::invalid_argument("Capacity must be > 0");
        }
    }

    ~LRUCache() { }

    LRUCache(LRUCache const& other) {
        std::scoped_lock lock(other.mutex);

        capacity = other.capacity;
        items = other.items;

        for (auto it = items.begin(); it != items.end(); ++it) {
            map[it->first] = it;
        }
    }

    LRUCache& operator=(LRUCache const& other) {
        if (this == &other) return *this;

        std::scoped_lock lock(mutex, other.mutex);

        capacity = other.capacity;
        items = other.items;

        map.clear();
        for (auto it = items.begin(); it != items.end(); ++it) {
            map[it->first] = it;
        }

        return *this;
    }

    LRUCache(LRUCache&& other) noexcept {
        std::scoped_lock lock(other.mutex);
        capacity = other.capacity;
        items = std::move(other.items);
        map = std::move(other.map);
        other.map.clear();
        other.items.clear();
    }

    LRUCache& operator=(LRUCache&& other) noexcept {
        if (this == &other) return *this;
        std::scoped_lock lock(mutex, other.mutex);
        capacity = other.capacity;
        items = std::move(other.items);
        map = std::move(other.map);
        other.map.clear();
        other.items.clear();
        return *this;
    }

    void put(K const& key, V const& value) {
        std::scoped_lock lock(mutex);
        auto it = map.find(key);
        if (it != map.end()) {
            it->second->second = value;                
            items.splice(items.begin(), items, it->second); 
            it->second = items.begin();
            return;
        }

        items.emplace_front(key, value);
        map[key] = items.begin();

        if (map.size() > capacity) {
            auto lru = items.end();
            --lru;
            map.erase(lru->first);
            items.pop_back();
        }
    }

    std::optional<V> get(K const& key) {
        std::scoped_lock lock(mutex);
        auto it = map.find(key);
        if (it == map.end()) return std::nullopt;
        items.splice(items.begin(), items, it->second);
        it->second = items.begin();
        return it->second->second;
    }

    size_t get_size() {
        std::scoped_lock lock(mutex);
        return map.size();
    }

    size_t get_capacity() {
        return capacity;
    }

private:

    using ListIt = typename std::list<std::pair<K, V>>::iterator;
    size_t capacity;
    std::list<std::pair<K, V>> items;
    std::unordered_map<K, ListIt> map;

    std::mutex mutex;
};


// =================== ТЕСТЫ ===================

void test_from_Artyom_Nikolaevich() {
    std::cout << "[test_from_Artyon_Nikolaevich] start" << std::endl;
    LRUCache<int, int> c(2);
    c.put(1, 1);
    c.put(2, 2);
    std::cout << c.get(1).value() << std::endl;
    c.put(2, 3);
    c.put(2, 4);
    std::cout << c.get(2).value() << std::endl;
    c.put(1, 2);
    c.put(3, 4);
    std::cout << c.get(2).has_value() << std::endl;
    c.put(4, 5);
    std::cout << c.get(1).has_value() << std::endl;
    std::cout << "[test_from_Artyon_Nikolaevich] end" << std::endl;
}

void test_basic_put_get() {
    std::cout << "[test_basic_put_get] start" << std::endl;
    LRUCache<int,int> c(2);
    c.put(1, 1);
    c.put(2, 2);

    std::cout << c.get(1).value() << std::endl;

    c.put(3, 3);
    std::cout << c.get(2).has_value() << std::endl;

    std::cout << c.get(3).value() << std::endl;

    std::cout << "[test_basic_put_get] passed" << std::endl;
}

void test_update_existing() {
    std::cout << "[test_update_existing] start" << std::endl;
    LRUCache<int,int> c(2);
    c.put(1, 1);
    std::cout << c.get(1).value() << std::endl;
    c.put(1, 10);
    std::cout << c.get(1).value() << std::endl;

    c.put(2, 2);
    std::cout << c.get(1).value() << std::endl;
    std::cout << c.get(2).value() << std::endl;

    std::cout << "[test_update_existing] passed" << std::endl;
}

void test_eviction_order() {
    std::cout << "[test_eviction_order] start" << std::endl;
    LRUCache<int,int> c(3);
    c.put(1,1);
    c.put(2,2);
    c.put(3,3);

    auto t = c.get(1);
    assert(t.has_value() && t.value() == 1);
    std::cout << c.get(1).value() << std::endl;

    c.put(4,4);
    std::cout << c.get(2).has_value() << std::endl;

    std::cout << "[test_eviction_order] passed\n";
}

void test_multithreaded_puts_and_gets() {
    std::cout << "[test_multithreaded_puts_and_gets] start\n";
    const size_t capacity = 50;
    LRUCache<int,int> cache(capacity);

    const int num_threads = 4;
    const int keys_per_thread = 100;
    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([t, &cache, keys_per_thread]() {
            int base = t * keys_per_thread;
            for (int i = 0; i < keys_per_thread; ++i) {
                int k = base + i;
                cache.put(k % 120, k);
                if (i % 10 == 0) {
                    cache.get(k % 120);
                }
            }
        });
    }

    for (auto &th : threads) th.join();

    assert(cache.get_size() <= capacity);
    std::cout << cache.get_size() << std::endl;

    for (int k = 0; k < 10; ++k) {
        (void)cache.get(k);
    }

    std::cout << "[test_multithreaded_puts_and_gets] passed (size=" << cache.get_size() << ")\n";
}

void test_rule_of_five() {
    std::cout << "[test_rule_of_five] start" << std::endl;

    LRUCache<int,int> c1(3);
    c1.put(1,10);
    c1.put(2,20);
    c1.put(3,30);

    LRUCache<int,int> c2(std::move(c1));

    std::cout << c2.get(1).value() << " " << c2.get(2).value() << " " << c2.get(3).value() << std::endl;

    LRUCache<int,int> c3(5);
    c3.put(100, 999);

    c3 = std::move(c2);

    std::cout << c3.get(100).has_value() << std::endl;

    std::cout << "[test_rule_of_five] passed" << std::endl;
}


void run_all_tests() {
    test_from_Artyom_Nikolaevich();
    test_basic_put_get();
    test_update_existing();
    test_eviction_order();
    test_multithreaded_puts_and_gets();
    test_rule_of_five();
    std::cout << "\nAll tests passed!\n";
}

int main() {
    run_all_tests();
    return 0;
}