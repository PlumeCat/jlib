// static_stack.h
#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>

template<typename T, size_t N> struct static_stack final : public std::array<T, N> {
    size_t count = 0;

    void push_back(const T& t) {
        if (count == N) {
            throw std::runtime_error("static_stack ran out of space");
        }
        (*this)[count++] = t;
    }

    void emplace_back(auto&&... args) {
        if (count == N) {
            throw std::runtime_error("static_stack ran out of space");
        }
        (*this)[count++] = T(std::forward<decltype(args)>(args)...);
    }

    void pop_back() {
        if (count == 0) {
            throw std::runtime_error("static_stack is empty");
        }
        count -= 1;
    }

    size_t size() const {
        return count;
    }
};
