// swiss_vector.h
#pragma once

#include <vector>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <initializer_list>

/*
swiss_vector<T>

a gap array. called "swiss" because it has holes in it like swiss cheese

- constant-ish time inserts and removes, while reusing free space

- iteration will be ever so slightly worse than linear
    but actually approaches linear as N -> ∞

- never invalidates indices, "remove()" will leave a gap rather than shuffling/swapping
    elements;
- invalidates pointers iff:
    * AllowResize
    and
    * add

*/

#include <iostream>

template<typename T, bool AllowResize = true>
class swiss_vector {
protected:
    std::vector<T> storage;
    std::vector<size_t> free_slots;
    std::vector<bool> is_busy;

public:
    using type = swiss_vector<T, AllowResize>;

    template<typename Container, typename Deref>
    struct iter {
        Container* container;
        size_t index;

        Deref& operator*() { return container->storage[index]; }
        Deref* operator->() { return &container->storage[index]; }
        bool operator != (const iter& i) const {
            return (container != i.container || index != i.index);
        }
        bool operator == (const iter& i) const {
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
    };

    using iterator = iter<type, T>;
    using const_iterator = iter<const type, const T>;

    swiss_vector() = delete;
    swiss_vector(const swiss_vector&) = default;
    swiss_vector(swiss_vector&&) = default;
    swiss_vector& operator=(const swiss_vector&) = default;
    swiss_vector& operator=(swiss_vector&&) = default;

    explicit swiss_vector(size_t capacity = 32) {
        is_busy.resize(capacity, false);
        storage.reserve(capacity);
        free_slots.reserve(capacity);
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

    T* index_to_pointer(size_t index) {
        return &storage[index];
    }
    size_t pointer_to_index(T* ptr) {
        return (ptr - storage.data()) / sizeof(T);
    }

    // returns the index where the value was added
    size_t add(const T& value) {
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
                if (storage.size() < storage.capacity()) {
                    is_busy[storage.size()] = true;
                    storage.emplace_back(value);
                    return storage.size() - 1;
                } else {
                    throw std::runtime_error("Full!");
                }
            }
        }
    }

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
    void clear() {
        storage.clear();
        free_slots.clear();
        is_busy.clear();
    }

    // swap [some of] the free slots to the end
    // improves locality and iteration speed at the expense of invalidating some indices
    // max_swaps = maximum number of swaps to perform (if -1, will compactify entirely)
    void compactify(int32_t max_swaps = -1);

    // get pointer to the raw data
    T* data() { return storage.data(); }

    // inquires whether the given slot is busy
    bool busy(size_t index) const { return is_busy[index]; }

    // the number of active elements
    size_t size() const { return storage.size(); }
    size_t count() const { return storage.size() - free_slots.size(); }

    // capacity of the internal vector; adding more than this number of
    // elements will invalidate pointers
    size_t capacity() const { return storage.capacity(); }

    // "begin" iterator contains the index of the first busy slot, or 0 if none
    iterator begin() {
        return find_begin(iterator { this, 0 });
    }
    const_iterator cbegin() const {
        return find_begin(const_iterator { this, 0 });
    }

    // "end" iterator contains the index of the last busy slot + 1, or 0 if none
    iterator end() {
        return find_end(iterator { this, storage.size() });
    }
    const_iterator cend() const {
        return find_end(const_iterator { this, storage.size() });
    }

private:
    template<typename Iter>
    Iter find_begin(Iter i) const {
        auto c = storage.size();
        while (i.index < c && !busy(i.index)) i.index++;
        return i;
    }

    template<typename Iter>
    Iter find_end(Iter i) const {
        while (i.index > 0 && !busy(i.index-1)) i.index--;
        return i;
    }
};
