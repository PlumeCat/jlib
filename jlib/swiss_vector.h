// swiss_vector.h
#pragma once

#include <cstdint>
#include <initializer_list>
#include <limits>
#include <stdexcept>
#include <vector>
#include <iostream>

/*
swiss_vector<T>

gap array
"swiss" because it has holes in it like swiss cheese

- constant-ish time inserts and removes, while reusing free space

- iteration will be ever so slightly worse than linear
    but actually approaches linear as N -> ∞

- never invalidates indices, "remove()" will leave a gap rather than shuffling/swapping
    elements;

- invalidates pointers iff:
    * AllowResize is true

*/


template<typename T, bool AllowResize = true> class swiss_vector {
private:
    template<typename Container, typename Deref> struct iter {
        Container* container;
        size_t index;

        Deref& operator*() {
            return container->storage[index];
        }
        Deref* operator->() {
            return &container->storage[index];
        }
        bool operator!=(const iter& i) const {
            return (container != i.container || index != i.index);
        }
        bool operator==(const iter& i) const {
            return (container == i.container && index == i.index);
        }
        iter& operator++() {
            auto c = container->storage.size();
            while (index < c && !container->busy(++index));
            return *this;
        }
        iter operator++(int) {
            auto t = *this;
            ++(*this);
            return t;
        }
        iter operator+(int n) {
            auto c = container->storage.size();
            auto t = *this;
            while (true) {
                if(index == c) {
                    break;
                }
                if (container->is_busy[index]) {
                    n -= 1;
                    if (n == 0) {
                        break;
                    }
                }
            }

            return t;
        }
    };

public:
    using type = swiss_vector<T, AllowResize>;

    using iterator = iter<type, T>;
    using const_iterator = iter<const type, const T>;

    swiss_vector() = default;
    swiss_vector(const swiss_vector&) = default;
    swiss_vector(swiss_vector&&) = default;
    swiss_vector& operator=(const swiss_vector&) = default;
    swiss_vector& operator=(swiss_vector&&) = default;

    // TODO: improve efficiency
    template<typename Rest>
    swiss_vector(std::initializer_list<Rest> elements) {
        reserve(elements.size());
        for (const auto& e : elements) {
            emplace_back(e);
        }
    }

    // TODO: improve efficiency
    template<typename... Args>
    swiss_vector(Args... args) {
        reserve(sizeof...(Args));
        (emplace_back(args), ...);
    }

    // Convenience method for getting all the live elements
    // TODO: improve efficiency
    std::vector<T> collect() {
        // return std::vector { begin(), end() };
        auto v = std::vector<T>{};
        for (auto i = 0u; i < storage.size(); i++) {
            if (is_busy[i]) {
                v.emplace_back(storage[i]);
            }
        }
        return v;
    }

    // get the element at index; don't assign to return value!
    // if not busy, it's whatever was in storage (most likely a default-constructed T, or a previously removed element)
    // T& at(size_t index) {
    //     return storage.at(index);
    // }

    // may not actually go in the back
    T& emplace_back(auto&&... args) {
        if (free_slots.size()) {
            // use a free slot if there is one
            auto index = free_slots.back();
            free_slots.pop_back();
            // assign value to slot
            storage[index] = T(args...);
            is_busy[index] = true;
            return storage[index];
        } else {
            if constexpr (AllowResize) {
                const auto old_cap = storage.capacity();
                storage.emplace_back(std::forward<decltype(args)>(args)...);
                const auto new_cap = storage.capacity();
                if (old_cap != new_cap) {
                    // a reallocation occurred; resize is_busy accordingly
                    is_busy.resize(new_cap, false);
                }
                is_busy[storage.size() - 1] = true;
                return storage.back();
            } else {
                if (storage.size() < storage.capacity()) {
                    is_busy[storage.size()] = true;
                    return storage.emplace_back(std::forward<decltype(args)>(args)...);
                } else {
                    throw std::runtime_error("swiss_vector is full");
                }
            }
        }
    }

    void reserve(size_t size) {
        if constexpr(AllowResize) {
            storage.reserve(size);
            free_slots.reserve(size);
            is_busy.resize(size, false);

}    }

    // remove the value at that index, and mark that slot as available
    void remove(size_t index) {
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

    template<typename Predicate>
    void remove_if(Predicate predicate) {
        auto s = size();
        for (auto i = 0; i < s; i++) {
            // if (busy(i) && predicate(at(i))) {
            //     remove(i);
            // }
        }
    }

    void clear() {
        storage.clear();
        free_slots.clear();
        std::fill(is_busy.begin(), is_busy.end(), false);
    }

    // swap free slots to the end
    // improves locality and iteration speed
    // invalidates iterators/indices/pointers
    // max_swaps = maximum number of swaps to perform (if -1, will compactify entirely)
    void compactify(int32_t max_swaps = -1);

    // get pointer to the raw data
    T* data() { return storage.data(); }
    const T* data() const { return storage.data(); }

    // inquires whether the given slot is busy
    bool busy(size_t index) const { return is_busy[index]; }

    // the number of active elements
    size_t size() const { return storage.size() - free_slots.size(); }

    // capacity of the internal storage; adding more than this number of
    // elements will invalidate pointers
    size_t capacity() const { return storage.capacity(); }

    // "begin" iterator contains the index of the first busy slot, or 0 if none
    iterator begin() { return find_begin(iterator { this, 0 }); }
    const_iterator cbegin() const { return find_begin(const_iterator { this, 0 }); }

    // "end" iterator contains the index of the last busy slot + 1, or 0 if none
    iterator end() { return find_end(iterator { this, storage.size() }); }
    const_iterator cend() const { return find_end(const_iterator { this, storage.size() }); }

private:
    template<typename Iter> Iter find_begin(Iter i) const {
        auto c = storage.size();
        while (i.index < c && !busy(i.index))
            i.index++;
        return i;
    }

    template<typename Iter> Iter find_end(Iter i) const {
        while (i.index > 0 && !busy(i.index - 1))
            i.index--;
        return i;
    }

    std::vector<T> storage;
    std::vector<size_t> free_slots;
    std::vector<bool> is_busy;
};

// CTAD
template<typename First, typename ...Rest> swiss_vector(First, Rest...) -> swiss_vector<First>;
