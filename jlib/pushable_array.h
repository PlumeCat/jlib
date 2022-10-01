// pushable_array.h
#pragma once

#include <array>

template<typename T, size_t N>
struct pushable_array final : public std::array<T, N> {;
    size_t count = 0;

    void push_back(const T& t) {
        if (count == N) {
            throw std::runtime_error("pushable_array ran out of space");
        }
        (*this)[count++] = t;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (count == N) {
            throw std::runtime_error("pushable_array ran out of space");
        }
        (*this)[count++] = T(std::forward<Args>(args)...);
    }

    void pop_back() {
        if (count == 0) {
            throw std::runtime_error("pushable_array is empty");
        }
        count -= 1;
    }

    size_t size() const {
        return count;
    }
};

