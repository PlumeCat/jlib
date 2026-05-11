// fixed_pool.h

#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <iterator>

template<typename Pool> struct pool_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Pool::value_type;
    using reference = Pool::value_type&;
    using pointer = Pool::value_type*;

    pool_iterator(Pool* pool, size_t index): pool(pool), index(index) { next(); }
    pool_iterator(const pool_iterator&) = default;
    pool_iterator(pool_iterator&&) = default;
    pool_iterator& operator=(const pool_iterator&) = default;
    pool_iterator& operator=(pool_iterator&&) = default;

    pool_iterator& operator++() { index++; next(); return *this; }
    pool_iterator operator++(int) { const auto that = *this; ++(*this); return that; }
    auto& operator*() const { return pool->get_storage().at(index); }
    auto* operator->() const { return &pool->get_storage().at(index); }
    bool operator==(const pool_iterator& i) { return index == i.index; }
    bool operator!=(const pool_iterator& i) { return index != i.index; }

private:
    void next() {
        while (index < pool->capacity() && !pool->is_busy(index)) {
            index++;
        }
    }

    // TODO: pointer + index means 2 indirections to dereference
    Pool* pool;
    size_t index;
};


template<typename T> class object_pool final {
public:
    using value_type = T;
    using iterator = pool_iterator<object_pool<T>>;
    using const_iterator = pool_iterator<const object_pool<T>>;

    object_pool();
    

    size_t capacity() const noexcept { return storage.capacity(); }
private:

    std::vector<T> storage;
};


// non growable object pool
// allocates space for all objects at initialization
// at first, items are added at the back of a vector
// when an item is removed, add the slot to a list of free slots
// result: ~O(1) insert, O(1) remove, slightly worse than O(1) iteration

template<typename T> class fixed_pool final {
public:
    using value_type = T;
    using iterator = pool_iterator<fixed_pool<T>>;
    using const_iterator = pool_iterator<const fixed_pool<T>>;

    explicit fixed_pool(size_t capacity) {
        storage.reserve(capacity);
        slot_busy.resize(capacity, false);
        free_slots.reserve(capacity);
    }
    fixed_pool(std::initializer_list<T> elements): fixed_pool(elements.size()) {
        auto i = 0;
        for (auto& e: elements) {
            slot_busy.at(i) = true;
            storage.emplace_back(e);
            i++;
        }
    }
    fixed_pool(const fixed_pool&) = default;
    fixed_pool(fixed_pool&&) = default;
    fixed_pool& operator=(const fixed_pool&) = default;
    fixed_pool& operator=(fixed_pool&&) = default;

    // TODO: growable pool

    T& add(auto&&... args) {
        if (count() == capacity()) {
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
    void remove(iterator i) {
        remove(*i);
        // slot_busy.at(i.index) = false;
        // if (i.index == storage.size() - 1) {
        //     storage.pop_back();
        // } else {
        //     free_slots.emplace_back(index);
        // }
    }
    void remove(const T& t) {
        const auto index = &t - storage.data();
        if (&t < storage.data() || index > capacity() || !slot_busy.at(index)) {
            return; // invalid element
        }
        slot_busy.at(index) = false;
        if (index == storage.size() - 1) {
            storage.pop_back();
        } else {
            free_slots.emplace_back(index);
        }
    }
    void remove_if(auto&& callable) {
        for (auto index = 0; index < capacity(); index++) {
            if (is_busy(index) && callable(storage.at(index))) {
                slot_busy.at(index) = false;
                free_slots.emplace_back(index);
            }
        }
    }
    void clear() {
        free_slots.clear();
        storage.clear();
        std::fill(slot_busy.begin(), slot_busy.end(), false);
    }

    size_t capacity() const noexcept { return storage.capacity(); }
    size_t count() const noexcept { return storage.size() - free_slots.size(); }
    bool is_busy(size_t index) const noexcept { return slot_busy.at(index); }
    const std::vector<T>& get_storage() const noexcept { return storage; }
    std::vector<T> collect() const { return std::vector<T> { begin(), end() }; }

    iterator begin() noexcept { return { this, 0u }; }
    iterator end() noexcept { return { this, capacity() }; }
    const_iterator cbegin() const noexcept { return { this, 0u }; }
    const_iterator cend() const noexcept { return { this, capacity() }; }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator end() const noexcept { return cend(); } 

private:
    std::vector<T> storage;
    std::vector<bool> slot_busy;
    std::vector<size_t> free_slots;
};

template<typename First, typename ...Rest> fixed_pool(First, Rest...) -> fixed_pool<First>;


/*
Notes:
    remove_if() naive approach does this:
        for index where predicate(storage[index])
            busy[index] = false
            free_slots << index

    ? Might leave several slots in the free_slots list
    unnecessarily (eg last N contiguous elements are removed)

    track the "run" of removed elements (ignoring free slots) and if we hit the end,
    during a run, erase from free slots and downsize storage

    S = [ 1, 2, 3, 4, 5, _, 7, 8, _, _ ]
        [-----------------------]......|
    F = [ 5 ]

    remove_if (x > 4)
        naive:
            S = [ 1, 2, 3, 4, _, _, _, _, _, _ ]
                [-----------------------]......|
            F = [ 5, 4, 6, 7 ]
        ideal:
            S = [ 1, 2, 3, 4, _, _, _, _, _, _ ]
                [-----------]..................|
            F = []

    Not worth fixing as the cost of extraneous free slots is basically zero

*/