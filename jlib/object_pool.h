// fixed_pool.h

#pragma once

#include <vector>
#include <algorithm>
#include <iterator>
#include <cstdint>

template<typename Pool, typename Iter> struct pool_iterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Iter::value_type;
    using reference = Iter::reference;
    using pointer = Iter::pointer;

    pool_iterator(Pool& pool, Iter iter): pool(pool), iter(iter) { next(); }
    pool_iterator(const pool_iterator&) = default;
    pool_iterator(pool_iterator&&) = default;
    pool_iterator& operator=(const pool_iterator&) = default;
    pool_iterator& operator=(pool_iterator&&) = default;

    pool_iterator& operator++() { iter++; next(); return *this; }
    pool_iterator operator++(int) { const auto that = *this; ++(*this); return that; }
    // pool_iterator operator+(int n) { auto that = *this; for (auto _ = 0; _ < n; _++) ++that; return that; }
    reference operator*() noexcept { return *iter; }
    pointer operator->() noexcept { return iter; }
    bool operator == (const pool_iterator& i) const noexcept { return iter == i.iter; }
    bool operator != (const pool_iterator& i) const noexcept { return iter != i.iter; }
    bool operator <  (const pool_iterator& i) const noexcept { return iter < i.iter; }

private:
    void next() {
        // TODO: use lzcnt over slot_busy array (convert to u64) to skip many at once
        while (iter < pool.storage.end() && !pool.is_busy(*this)) {
            iter++;
        }
    }

    Pool& pool;
    Iter iter;

    friend Pool;
};

// object pool
// allocates space for all objects at initialization
// at first, items are added at the back of a vector
// when an item is removed, add the slot to a list of free slots
// result: ~O(1) insert, O(1) remove, slightly worse than O(1) iteration
template<typename T, bool Resizable = false, size_t DefaultCapacity = 0u> class object_pool {
public:
    using this_type = object_pool<T, Resizable, DefaultCapacity>;
    using value_type = T;
    using iterator = pool_iterator<this_type, typename std::vector<T>::iterator>;
    using const_iterator = pool_iterator<const this_type, typename std::vector<T>::const_iterator>;

    explicit object_pool(size_t capacity = DefaultCapacity) {
        if constexpr (!Resizable) {
            if (capacity == 0u) {
                throw std::runtime_error { "fixed_pool: non-resizable with capacity 0 => unusable!" };
            }
        }
        storage.reserve(capacity);
        slot_busy.resize(capacity, false);
        free_slots.reserve(capacity);
    }
    object_pool(std::initializer_list<T> elements): object_pool(elements.size()) {
        std::copy(elements.begin(), elements.end(), std::back_inserter(storage));
        std::fill(slot_busy.begin(), slot_busy.begin() + elements.size(), true);
    }
    object_pool(auto&&... args): object_pool(sizeof...(args)) {
        (add(std::forward<decltype(args)>(args)), ...);
    }
    object_pool(const object_pool&) = default;
    object_pool(object_pool&&) = default;
    object_pool& operator=(const object_pool&) = default;
    object_pool& operator=(object_pool&&) = default;

    // put an element; return an iterator to the element
    // if fixed size is exceeded, throw exception
    iterator emplace(auto&&... args) {
        auto index = -1uz;
        if (free_slots.size()) {
            index = free_slots.back();
            free_slots.pop_back();
            storage.at(index) = T { std::forward<decltype(args)>(args)... };
        } else {
            if constexpr (Resizable) {
                index = storage.size();
                const auto old_cap = storage.capacity();
                storage.emplace_back(std::forward<decltype(args)>(args)...);
                const auto new_cap = storage.capacity();
                if (new_cap != old_cap) {
                    slot_busy.resize(new_cap, false);
                    free_slots.reserve(new_cap);
                }
            } else {
                if (storage.size() == storage.capacity()) {
                    throw std::runtime_error { "fixed pool full!" };
                }
                index = storage.size();
                storage.emplace_back(std::forward<decltype(args)>(args)...);
            }
        }
        slot_busy.at(index) = true;
        return { *this, storage.begin() + index };
    }

    // like emplace, but returns the index instead
    size_t add(auto&&... args) {
        return index_of(emplace(std::forward<decltype(args)>(args)...));
    }

    // make sure there is enough space for size elements
    // may move elements
    // does nothing if not resizable
    void reserve(size_t size) {
        if constexpr(Resizable) {
            free_slots.reserve(size);
            storage.reserve(size);
            slot_busy.resize(size, false);
        }
    }

    // return index for this iterator
    // the slot may not be occupied, so at() will error
    size_t index_of(iterator i) const {
        return std::distance(storage.begin(), typename decltype(storage)::const_iterator(i.iter));
    }
    size_t index_of(const_iterator i) const {
        return std::distance(storage.cbegin(), i.iter);
    }

    // remove element pointed to by this iterator
    void remove(iterator i) {
        remove(index_of(i));
    }

    // remove element at the given index
    void remove(size_t index) {
        if (index > storage.size() || !is_busy(index)) { return; }

        /**/storage.at(index) = T('_');
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


    // throws on out of range or if slot is not occupied
    decltype(auto) at(this auto& self, size_t index) {
        if (!self.slot_busy.at(index)) throw std::runtime_error { "invalid element" };
        return self.at_fast(index);
    }

    // throws on out of range;
    // can return removed or invalid element if slot is not occupied
    decltype(auto) at_fast(this auto& self, size_t index) {
        return self.storage.at(index);
    }
    
    // compact all elements into a vector
    std::vector<T> collect() const {
        return std::vector<T> { begin(), end() };
    }

    /**/ std::vector<T> collect2() const {
    /**/     return storage;
    /**/ }

    size_t capacity() const noexcept { return storage.capacity(); }
    size_t width() const noexcept { return storage.size(); }
    size_t count() const noexcept { return storage.size() - free_slots.size(); }
    bool is_busy(size_t index) const noexcept { return slot_busy.at(index); }
    bool is_busy(iterator i) const noexcept { return slot_busy.at(index_of(i)); }
    bool is_busy(const_iterator i) const noexcept { return slot_busy.at(index_of(i)); }

    // iterators
    iterator        begin()       noexcept { return { *this, storage.begin() }; }
    iterator        end()         noexcept { return { *this, storage.end() }; }
    const_iterator  begin() const noexcept { return { *this, storage.cbegin() }; }
    const_iterator  end()   const noexcept { return { *this, storage.cend() }; }
    const_iterator  cbegin()const noexcept { return begin(); }
    const_iterator  cend()  const noexcept { return end(); }

private:
    std::vector<T> storage;
    std::vector<bool> slot_busy;
    std::vector<size_t> free_slots;

    friend iterator;
    friend const_iterator;
};

template<typename T> class fixed_pool : public object_pool<T, false, 64u> {
    using base = object_pool<T, false, 64u>;
    using base::base;
};
template<typename T> class dynamic_pool : public object_pool<T, true> {
    using base = object_pool<T, true>;
    using base::base;
};
template<typename First, typename ...Rest> fixed_pool(First, Rest...) -> fixed_pool<First>;
template<typename First, typename ...Rest> dynamic_pool(First, Rest...) -> dynamic_pool<First>;


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
