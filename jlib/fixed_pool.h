// fixed_pool.h

#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <iterator>

#define debug(...)
#define debug(...) log("DEBUG", __VA_ARGS__)

// non growable object pool
// allocates space for all objects at initialization
// at first, items are added at the back of a vector
// when an item is removed, add the slot to a list of free slots
// result: ~O(1) insert, O(1) remove, slightly worse than O(1) iteration

template<typename T> class fixed_pool final {
public:
    // static const auto BUSY = 1u;
    // static const auto FREE = 0u;
    template<typename Pool, typename Elem> struct iter {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = Elem;
        using pointer = Elem*;
        using reference = Elem&;

        Pool* pool;
        size_t index;

        iter(Pool* pool, size_t index): pool(pool), index(index) { next(); }

        iter& operator++() { index++; next(); return *this; }
        iter operator++(int) { const auto that = *this; ++(*this); return that; }

        reference operator*() const { return pool->get_storage().at(index); }
        pointer operator->() const { return &pool->get_storage().at(index); }

        friend bool operator==(const iter& a, const iter& b) { return a.index == b.index; }
        friend bool operator!=(const iter& a, const iter& b) { return a.index != b.index; }

    private:
        void next() {
            while (index < pool->total_capacity && !pool->is_busy(index)) {
                index++;
            }
        }
    };

    using this_type = fixed_pool<T>;
    using iterator = iter<this_type, T>;
    using const_iterator = iter<const this_type, const T>;

    fixed_pool(size_t capacity):
        total_capacity(capacity) {
        storage.reserve(capacity);
        slot_busy.resize(capacity, false);
        free_slots.reserve(capacity);
    }
    fixed_pool(const fixed_pool&) = default;
    fixed_pool(fixed_pool&&) = default;
    fixed_pool& operator=(const fixed_pool&) = default;
    fixed_pool& operator=(fixed_pool&&) = default;

    T& add(auto&&... args) {
        if (count() == total_capacity) {
            throw std::runtime_error { "fixed_pool full!" };
        }
        if (free_slots.size()) {
            const auto index = free_slots.back();
            free_slots.pop_back();
            slot_busy.at(index) = true;
            storage.at(index) = T { std::forward<decltype(args)>(args)... };
            return storage.at(index);
        } else {
            slot_busy.at(storage.size()) = true;
            return storage.emplace_back(std::forward<decltype(args)>(args)...);
        }
    }
    void remove(const T& t) {
        if (&t < storage.data()) {
            return; // invalid element
        }
        
        const auto index = &t - storage.data();
        if (index > total_capacity) {
            return; // invalid element
        }
        if (!slot_busy.at(index)) {
            return; // already free
        }

        slot_busy.at(index) = false;
        if (index == storage.size() - 1) {
            storage.pop_back();
        } else {
            free_slots.emplace_back(index);
        }
    }
    void clear() {
        free_slots.clear();
        storage.clear();
        std::fill(slot_busy.begin(), slot_busy.end(), false);
    }

    size_t capacity() const noexcept {
        return total_capacity;
    }
    size_t count() const noexcept {
        return storage.size() - free_slots.size();
    }
    bool is_busy(size_t index) const noexcept {
        return slot_busy.at(index);
    }
    const std::vector<T>& get_storage() const noexcept {
        return storage;
    }

    std::vector<T> collect() const {
        return std::vector<T> { begin(), end() };
    }

    iterator begin() noexcept { return { this, 0u }; }
    iterator end() noexcept { return { this, total_capacity }; }
    const_iterator cbegin() const noexcept { return { this, 0u }; }
    const_iterator cend() const noexcept { return { this, total_capacity }; }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator end() const noexcept { return cend(); } 

private:
    std::vector<T> storage;
    std::vector<bool> slot_busy;
    std::vector<size_t> free_slots;
    size_t total_capacity;
};
