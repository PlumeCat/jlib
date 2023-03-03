// swiss_vector.h
#pragma once

#include <vector>
#include <initializer_list>

/*
swiss_vector<T>

a gap array. called "swiss" because it has holes in it like swiss cheese
behaves.

- constant-ish time inserts and removes, while reusing free space

- iteration will be ever so slightly worse than linear
    but actually approaches linear as N -> ∞

- never invalidates indices, "remove()" will leave a gap rather than shuffling/swapping
    elements; 
- invalidates pointers if "add()"

*/


template<typename T, bool AllowResize = true>
class swiss_vector {
    template<class Iter, class Busy> struct iter_base {
        Iter storage_iter;
        Busy busy_iter;

        T& operator*() { return *storage_iter; }
        T* operator->() { return &(*storage_iter); }
        bool operator != (const iter_base& i) const { return storage_iter != i.storage_iter; }
        bool operator == (const iter_base& i) const { return storage_iter == i.storage_iter; }
        iter_base& operator++()   {
            while (!*busy_iter) {
                storage_iter++;
                busy_iter++;
            }
            return *this;
        }
        iter_base operator++(int) {
            auto t = *this;
            ++(*this);
            return t;
        }

    };

protected:
    std::vector<T> storage;
    std::vector<size_t> free_slots;
    std::vector<bool> is_busy;

public:
    using iterator = iter_base<decltype(storage.begin()), decltype(is_busy.begin())>;
    using const_iterator = iter_base<decltype(storage.cbegin()), decltype(is_busy.cbegin())>;

    swiss_vector() = delete;
    swiss_vector(const swiss_vector&) = default;
    swiss_vector(swiss_vector&&) = default;
    swiss_vector& operator=(const swiss_vector&) = default;
    swiss_vector& operator=(swiss_vector&&) = default;

    swiss_vector(size_t capacity = 32) {
        is_busy.resize(capacity, false);
        storage.reserve(capacity);
    }

    // TODO: improve efficiency
    template<typename... Rest>
    swiss_vector(const std::initializer_list<Rest...>& elements):
        swiss_vector(elements.size()) {
        for (const auto& e: elements) {
            add(e);
        }
    }

    // TODO: improve efficiency
    template<typename ...Rest>
    swiss_vector(Rest... rest): swiss_vector(sizeof...(Rest)) {
        add(rest...);
    }


    // TODO: improve efficiency
    template<typename ...Rest>
    void add(const T& value, const Rest& ...rest) {
        add(value);
        add(rest...);
    }

    // Convenience method for getting all the live elements
    // TODO: improve efficiency
    std::vector<T> collect() {
        auto v = std::vector<T>();
        for (auto i = 0; i < storage.size(); i++) {
            if (is_busy[i]) {
                v.emplace_back(storage[i]);
            }
        }
        return v;
    }

    T* index_to_pointer(uint32_t index) {
        return &storage[index];
    }
    uint32_t pointer_to_index(T* ptr) {
        return (ptr - storage.data()) / sizeof(T);
    }

    uint32_t add(const T& value) {
        if (free_slots.size()) {
            // use a free slot if possible to minimize gaps
            auto index = free_slots.back();
            free_slots.pop_back();
            // assign value to slot
            storage[index] = value;
            is_busy[index] = true;
            return index;
        } else {
            if constexpr (AllowResize) {
                auto old_cap = storage.capacity();
                storage.emplace_back(value);
                auto new_cap = storage.capacity();
                if (old_cap != new_cap) {
                    // a re-allocation occurred; resize is_busy accordingly
                    is_busy.resize(new_cap, false);
                }
                is_busy[storage.size() - 1] = true;
                return storage.size() - 1;
            } else {
                if (storage.size() < storage.capacity) {
                    is_busy[storage.size()] = true;
                    storage.emplace_back(value);
                    return storage.size();
                } else {
                    throw std::runtime_error("Full!");
                }
            }
        }
    }
    void remove(uint32_t index) {
        // remove an element that isn't there does nothing
        if (is_busy[index]) {
            is_busy[index] = false;
            if (index == storage.size() - 1) {
                // better not to record free slots past the end of storage
                storage.pop_back();
            } else {
                free_slots.emplace_back(index);
            }
        }
    }
    void clear() {
        storage.clear();
        free_slots.clear();
        is_busy.clear();
    }

    // TODO: maybe? compressing feature: swap elemnents into free slots
    // to improve locality -> improve iteration speed
    // at the expense of invalidating some indices
    void compactify(uint32_t max_swaps = std::numeric_limits<uint32_t>::max());

    T* data() { return storage.data(); }
    bool busy(uint32_t index) { return is_busy[index]; }
    uint32_t size() const { return storage.size(); }
    uint32_t count() const { return storage.size() - free_slots.size(); }
    uint32_t capacity() const { return storage.capacity(); }

    iterator begin() {
        return iterator { storage.begin(), is_busy.begin() };
    }
    const_iterator begin() const {
        return const_iterator { storage.begin(), is_busy.begin() };
    }
    iterator end() {
        return iterator { storage.end(), is_busy.end() };
    }
    const_iterator end() const {
        return const_iterator { storage.begin(), is_busy.begin() };
    }

    // uint32_t count; // number of active elements
};
