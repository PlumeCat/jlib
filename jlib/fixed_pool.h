// fixed_pool.h

#pragma once

#include <vector>
#include <algorithm>
#include <iterator>

template<typename Pool, typename Type> struct pool_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Type;
    using reference = Type&;
    using pointer = Type*;

    pool_iterator(Pool& pool, Type* ptr): pool(pool), ptr(ptr) { next(); }
    pool_iterator(const pool_iterator&) = default;
    pool_iterator(pool_iterator&&) = default;
    pool_iterator& operator=(const pool_iterator&) = default;
    pool_iterator& operator=(pool_iterator&&) = default;

    pool_iterator& operator++() { ptr++; next(); return *this; }
    pool_iterator operator++(int) { const auto that = *this; ++(*this); return that; }
    Type& operator*() noexcept { return *ptr; }
    Type* operator->() noexcept { return ptr; }
    bool operator==(const pool_iterator& i) const noexcept { return ptr == i.ptr; }
    bool operator!=(const pool_iterator& i) const noexcept { return ptr != i.ptr; }

private:
    void next() {
        while (ptr < pool.data() + pool.capacity() && !pool.is_busy(ptr))
            ptr++;
    }

    Pool& pool;
    Type* ptr;
};

// non growable object pool
// allocates space for all objects at initialization
// at first, items are added at the back of a vector
// when an item is removed, add the slot to a list of free slots
// result: ~O(1) insert, O(1) remove, slightly worse than O(1) iteration

// TODO: make growable

template<typename T> class fixed_pool final {
public:
    using value_type = T;
    using iterator = pool_iterator<fixed_pool<T>, T>;
    using const_iterator = pool_iterator<const fixed_pool<T>, const T>;

    explicit fixed_pool(size_t capacity) {
        storage.reserve(capacity);
        slot_busy.resize(capacity, false);
        free_slots.reserve(capacity);
    }
    fixed_pool(std::initializer_list<T> elements): fixed_pool(elements.size()) {
        std::copy(elements.begin(), elements.end(), std::back_inserter(storage));
        std::fill(slot_busy.begin(), slot_busy.begin() + elements.size(), true);
    }
    fixed_pool(const fixed_pool&) = default;
    fixed_pool(fixed_pool&&) = default;
    fixed_pool& operator=(const fixed_pool&) = default;
    fixed_pool& operator=(fixed_pool&&) = default;

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
    void remove(const T& t) {
        const auto ptr = &t;
        if (ptr < storage.data() || ptr > storage.data() + storage.capacity()) {
            return;
        }
        const auto index = ptr - storage.data();
        if (!slot_busy.at(index)) {
            return;
        }
        slot_busy.at(index) = false;
        if (index == storage.size() - 1) {
            storage.pop_back();
        } else {
            free_slots.emplace_back(index);
        }
    }
    void remove_if(auto&& callable) {
        for (auto index = 0u; index < capacity(); index++) {
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
    bool is_busy(const T* ptr) const noexcept { return is_busy(ptr - data()); }

    decltype(auto) data(this auto& self) noexcept { return self.storage.data(); }
    // throws on out of range or if index is not busy
    decltype(auto) at(this auto& self, size_t index) {
        if (!self.slot_busy.at(index)) throw std::runtime_error { "invalid element" };
        return self.at_fast(index);
    }
    // throws on out of range; can return removed or invalid element if index is not busy
    decltype(auto) at_fast(this auto& self, size_t index) {
        return self.storage.at(index);
    }
    
    // collect all elements into a vector
    std::vector<T> collect() const { return std::vector<T> { begin(), end() }; }

    iterator begin() noexcept { return { *this, data() }; }
    iterator end() noexcept { return { *this, &data()[capacity()] }; }
    const_iterator begin() const noexcept { return cbegin(); }
    const_iterator end() const noexcept { return cend(); } 
    const_iterator cbegin() const noexcept { return { *this, data() }; }
    const_iterator cend() const noexcept { return { *this, &data()[capacity()] }; }

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

    Not worth fixing as the cost of extraneous free slots is negligible

*/