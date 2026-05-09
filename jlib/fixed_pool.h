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
// when an object is added,

template<typename T> class fixed_pool final {
public:
    enum class State : uint8_t { Free, Busy };
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

        reference operator*() const { return pool->storage.at(index); }
        pointer operator->() const { return &pool->storage.at(index); }

        friend bool operator==(const iter& a, const iter& b) { return a.index == b.index; }
        friend bool operator!=(const iter& a, const iter& b) { return a.index != b.index; }

    private:
        void next() {
            while (index < pool->total_capacity && pool->state.at(index) != State::Busy) {
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
        state.resize(capacity, State::Free);
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
            state.at(index) = State::Busy;
            storage.at(index) = T { std::forward<decltype(args)>(args)... };
            return storage.at(index);
        } else {
            state.at(storage.size()) = State::Busy;
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
        if (state.at(index) == State::Free) {
            return; // already free
        }

        state.at(index) = State::Free;
        if (index == storage.size() - 1) {
            storage.pop_back();
        } else {
            free_slots.emplace_back(index);
        }
    }
    void clear() {
        free_slots.clear();
        storage.clear();
        std::fill(state.begin(), state.end(), State::Free);
    }

    size_t capacity() const noexcept {
        return total_capacity;
    }
    size_t count() const noexcept {
        return storage.size() - free_slots.size();
    }

    iterator begin() { return { this, 0u }; }
    iterator end() { return { this, total_capacity }; }

// private:
    std::vector<T> storage;
    std::vector<State> state;
    std::vector<size_t> free_slots;
    size_t total_capacity;
    friend struct iter<this_type, T>;
};
