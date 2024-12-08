#ifndef _HEAPSORT_H
#define _HEAPSORT_H

#include <array>
#include <compare>
#include <exception>
#include <vector>

#include "static_stack.h"

template<typename Type, typename Compare, typename Storage> struct heap {
    Storage store;
    Compare compare;

    size_t get_count() const {
        return store.size();
    }

    // push a new element to the heap
    void push(Type t) {
        store.emplace_back(t);
        sift_up(get_count() - 1);
    }

    // pop an element
    Type pop() {
        if (get_count() == 0) {
            throw std::runtime_error("heap is empty");
        }

        // return the top element
        auto top = store[0];

        // move the bottom element to the top and sift it down
        swap_elements(get_count() - 1, 0);
        store.pop_back();
        sift_down(0);

        return top;
    }

private:
    void swap_elements(size_t i1, size_t i2) {
        std::swap(store[i1], store[i2]);
    }

    // sift up the element at index until it satisfies heap condition (<= parent, >= child)
    // tail recursive with no destructore, compiler should optimize into loop
    void sift_up(size_t index) {
        if (index >= get_count() || index == 0) {
            return;
        }
        auto parent_index = (index - 1) / 2;
        if (compare(store[index], store[parent_index])) {
            swap_elements(index, parent_index);
            index = parent_index;
            sift_up(parent_index);
        }
    }

    // sift down the element at index until it satisfies heap condition (<= parent, >= child)
    // tail recursive with no destructors, compiler should optimize into loop
    void sift_down(size_t index) {
        if (index >= get_count()) {
            return;
        }
        auto child1_index = index * 2 + 1;
        auto child2_index = child1_index + 1;
        auto next_index = child1_index; // 1 child case default

        if (child1_index > get_count() - 1) {
            return; // 0 child case
        } else if (child2_index <= get_count() - 1) {
            // 2 children; the larger child should sift up
            next_index = compare(store[child1_index], store[child2_index]) ? child1_index : child2_index;
        }

        // compare with next_index
        if (compare(store[index], store[next_index])) {
            return;
        } else {
            swap_elements(index, next_index);
            sift_down(next_index);
        }
    }
};

// dynamic heap
template<typename Type, typename Compare = std::greater<Type>> using dynamic_heap = heap<Type, Compare, std::vector<Type>>;

// static heap
template<typename Type, size_t N, typename Compare = std::greater<Type>>
using static_heap = heap<Type, Compare, static_stack<Type, N>>;

// heapsort for array
template<typename Type, size_t N, typename Compare = std::greater<Type>>
void heapsort(const std::array<Type, N>& input, std::array<Type, N>& output) {
    auto h = static_heap<Type, N, Compare> {};
    for (auto i : input) {
        h.push(i);
    }
    for (auto i = 0u; i < N; i++) {
        output[i] = h.pop();
    }
}

// heapsort for vector
template<typename Type, typename Compare = std::greater<Type>>
void heapsort(const std::vector<Type>& input, std::vector<Type>& output) {
    auto h = dynamic_heap<Type, Compare> {};
    for (auto& i : input) {
        h.push(i);
    }
    for (auto i = 0u; i < input.size(); i++) {
        output.emplace_back(h.pop());
    }
}

#endif
