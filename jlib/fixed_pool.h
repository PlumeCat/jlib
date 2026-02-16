// fixed_pool.h

#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <iterator>

// non growable object pool
// allocates space for all objects at initialization
// when an object is added,
template<typename T> class fixed_pool {
    template<typename E> struct iter {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = E;
        using pointer = E*;
        using reference = E&;

        iter(const fixed_pool& container, pointer ptr): container(container), ptr(ptr) {}

        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }

        iter& operator++() { next(); return *this; }
        iter operator++(int) { const auto that = *this; ++(*this); return that; }

        friend bool operator==(const iter& a, const iter& b) { return a.ptr == b.ptr; }
        friend bool operator!=(const iter& a, const iter& b) { return a.ptr != b.ptr; }

    private:
        const fixed_pool& container;
        pointer ptr;

        void next() {
            ptr++;
            if (ptr > container.last_valid) {
                ptr = container.last_valid + 1;
                return;
            }
            auto index = ptr - container.storage.data();
            if (container.state.at(index) == State::Free) {
                next();
            }
        }
    };

public:
    enum class State : uint8_t { Free, Busy };

    using iterator = iter<T>;
    using const_iterator = iter<const T>;

    fixed_pool(size_t capacity) {
        storage.reserve(capacity);
        free.reserve(capacity);
        state.resize(capacity, State::Free);
        first_valid = nullptr;
        last_valid = nullptr;
    }
    ~fixed_pool() = default;

    iterator begin() { return { *this, first_valid }; }
    iterator end() { return { *this, last_valid + 1 }; }    
    const_iterator cbegin() const { return { *this, first_valid }; }
    const_iterator cend() const { return { *this, last_valid + 1 }; }
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }

    T& add(auto&&... args) {
        // return null if no space
        if (count() == storage.capacity()) {
            throw std::runtime_error("fixed_pool is full!");
        }

        auto slot = 0;
        if (free.size()) {
            // use free slot
            slot = free.back(); free.pop_back();
            storage.at(slot) = T { std::forward<decltype(args)>(args)... };

            // if slot was before first valid, first
        } else {
            // add to back
            slot = storage.size();
            storage.emplace_back(std::forward<decltype(args)>(args)...);
        }

        state.at(slot) = State::Busy;

        // update first, last
        auto ptr = &storage.at(slot);
        if (!last_valid || ptr < first_valid) { first_valid = ptr; }
        if (!last_valid || ptr > last_valid) { last_valid = ptr; }

        return storage.at(slot);
    }

    bool remove(T* element) {
        if (storage.empty()) {
            return false;
        }

        if (element < first_valid || element > last_valid) {
            return false; // not a pointer to storage element
        }

        if (const auto index = element - storage.data(); state.at(index) == State::Busy) {
            // live element
            state.at(index) = State::Free;
            if (index == storage.size() - 1) {
                // last element special case
                storage.pop_back();
            } else {
                free.emplace_back(index);
            }

            if (count() == 0) {
                first_valid = nullptr;
                last_valid = nullptr;
            } else {
                if (element == first_valid) {
                    // scan forwards for next valid element
                    for (auto i = index; i < state.size(); i++) {
                        if (state.at(index) == State::Busy) {
                            first_valid = storage.data() + index;
                            break;
                        }
                    }
                }
                if (element == last_valid) {
                    // scan backwards for next valid element
                    for (auto i = index; i --> 0;) {
                        if (state.at(index) == State::Busy) {
                            last_valid = storage.data() + index;
                            break;
                        }
                    }
                }
            }
        }

        return true;
    }

    void clear() {
        storage.clear();
        free.clear();
        std::fill(state.begin(), state.end(), State::Free);
        first_valid = nullptr;
        last_valid = nullptr;
    }


    bool is_busy(size_t index) const { return state.at(index) == State::Busy; }
    size_t count_free_before(size_t index) const { auto f = 0; for (auto i = 0; i < index; i++) { if (!is_busy(i)) f++; } return f; }

    size_t capacity() const { return storage.capacity(); }
    bool has_space() const { return count() != capacity(); }
    uint32_t count() const { return storage.size() - free.size(); }
    uint32_t gaps() const { return free.size(); }

private:
    std::vector<T> storage;
    std::vector<State> state;
    std::vector<size_t> free;

    T* first_valid;
    T* last_valid;
};
