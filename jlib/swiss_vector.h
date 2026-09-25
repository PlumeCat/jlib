// // swiss_vector.h
// #pragma once

// #include <cstddef>
// #include <cstdint>
// #include <initializer_list>
// #include <iterator>
// #include <limits>
// #include <stdexcept>
// #include <vector>
// #include <iostream>

// /*
// swiss_vector<T>

// gap array
// "swiss" because it has holes in it like swiss cheese

// - constant-ish time inserts and removes, while reusing free space

// - iteration will be ever so slightly worse than linear
//     but actually approaches linear as N -> ∞

// - never invalidates indices, "remove()" will leave a gap rather than shuffling/swapping
//     elements;

// - invalidates pointers iff:
//     * AllowResize is true

// */


// template<typename Container, typename T> struct swiss_vector_iter {
//     using iterator_category = std::forward_iterator_tag;
//     using value_type = T;
//     using pointer = T*;
//     using reference = T&;
//     using difference_type = std::ptrdiff_t;

//     const Container& container;
//     size_t index;

//     swiss_vector_iter(Container& container, size_t index): container(container), index(index) {
//         if (!container->busy(index)) ++(*this);
//     }

//     reference operator*() {
//         return container->data()[index];
//     }
//     pointer operator->() {
//         return &container->data()[index];
//     }
//     bool operator!=(const swiss_vector_iter& i) const {
//         return (container != i.container || index != i.index);
//     }
//     bool operator==(const swiss_vector_iter& i) const {
//         return (container == i.container && index == i.index);
//     }

//     swiss_vector_iter& operator++() {
//         index = container->next(index);
//         return *this;
//     }
//     swiss_vector_iter operator++(int) {
//         auto t = *this;
//         ++(*this);
//         return t;
//     }
// };
// template<typename T, bool AllowResize = true> class swiss_vector {
// public:
//     using type = swiss_vector<T, AllowResize>;
//     using iterator = swiss_vector_iter<type, T>;
//     using const_iterator = swiss_vector_iter<const type, const T>;

//     swiss_vector() = default;
//     swiss_vector(const swiss_vector&) = default;
//     swiss_vector(swiss_vector&&) = default;
//     swiss_vector& operator=(const swiss_vector&) = default;
//     swiss_vector& operator=(swiss_vector&&) = default;

//     // TODO: improve efficiency
//     template<typename Rest>
//     swiss_vector(std::initializer_list<Rest> elements) {
//         reserve(elements.size());
//         for (const auto& e : elements) {
//             emplace_back(e);
//         }
//     }

//     // TODO: improve efficiency
//     swiss_vector(auto&&... args) {
//         reserve(sizeof...(args));
//         (emplace_back(std::forward<decltype(args)>(args)), ...);
//     }

//     // Convenience method for getting all the live elements
//     // TODO: improve efficiency
//     std::vector<T> collect() const {
//         auto b = begin();
//         auto e = end();
//         std::cout << "collect: " << b.index << ", " << e.index;
//         return std::vector(begin(), end());
//     }

//     // get the element at index
//     // if not busy, it's whatever was in storage (most likely a default-constructed T, or a previously removed element)
//     const T& at(size_t index) const {
//         return storage.at(index);
//     }

//     T& emplace_back(auto&&... args) {
//         if (free_slots.size()) {
//             // use a free slot if there is one
//             auto index = free_slots.back();
//             free_slots.pop_back();
//             // assign value to slot
//             storage[index] = T(std::forward<decltype(args)>(args)...);
//             is_busy[index] = true;
//             return storage[index];
//         } else {
//             if constexpr (AllowResize) {
//                 const auto old_cap = storage.capacity();
//                 storage.emplace_back(std::forward<decltype(args)>(args)...);
//                 const auto new_cap = storage.capacity();
//                 if (old_cap != new_cap) {
//                     // a reallocation occurred; resize is_busy accordingly
//                     is_busy.resize(new_cap, false);
//                 }
//                 is_busy[storage.size() - 1] = true;
//                 return storage.back();
//             } else {
//                 if (storage.size() < storage.capacity()) {
//                     is_busy[storage.size()] = true;
//                     return storage.emplace_back(std::forward<decltype(args)>(args)...);
//                 } else {
//                     throw std::runtime_error("swiss_vector is full");
//                 }
//             }
//         }
//     }

//     void reserve(size_t size) {
//         if constexpr(AllowResize) {
//             storage.reserve(size);
//             free_slots.reserve(size);
//             is_busy.resize(size, false);
//         }
//     }

//     // remove the value at that index, and mark that slot as available
//     void remove(size_t index) {
//         // remove an element that isn't there does nothing
//         if (busy(index)) {
//             is_busy[index] = false;
//             if (index == storage.size() - 1) {
//                 // better not to record free slots past the end of storage
//                 storage.pop_back();
//             } else {
//                 free_slots.emplace_back(index);
//             }
//         }
//     }

//     void remove_if(auto predicate) {
//         auto s = storage.size();
//         for (auto i = 0u; i < s; i++) {
//             if (busy(i) && predicate(at(i))) {
//                 remove(i);
//             }
//         }
//     }

//     void clear() {
//         storage.clear();
//         free_slots.clear();
//         std::fill(is_busy.begin(), is_busy.end(), false);
//     }

//     // swap free slots to the end
//     // improves locality and iteration speed
//     // invalidates iterators/indices/pointers
//     // max_swaps = maximum number of swaps to perform (if -1, will compactify entirely)
//     void compactify(int32_t max_swaps = -1);

//     // get pointer to the raw data
//     // T* data() { return storage.data(); }
//     const T* data() const { return storage.data(); }

//     // inquires whether the given slot is busy
//     bool busy(size_t index) const { return is_busy.at(index); }

//     // the number of active elements
//     size_t size() const { return storage.size() - free_slots.size(); }
//     size_t storage_size() const { return storage.size(); }

//     // capacity of the internal storage; adding more than this number of
//     // elements will invalidate pointers
//     size_t capacity() const { return storage.capacity(); }

//     // "begin" iterator contains the index of the first busy slot, or 0 if none
//     iterator begin() { return iterator { this, 0 }; }
//     const_iterator begin() const { return const_iterator { this, 0 }; }
//     const_iterator cbegin() const { return begin(); }

//     // "end" iterator contains the index of the last busy slot + 1, or 0 if none
//     iterator end() { return iterator { this, storage_size() }; }
//     const_iterator end() const { return const_iterator { this, storage_size() }; }
//     const_iterator cend() const { return end(); }

// protected:
//     std::vector<T> storage;
//     std::vector<size_t> free_slots;
//     std::vector<bool> is_busy;
// };

// // CTAD
// template<typename First, typename ...Rest> swiss_vector(First, Rest...) -> swiss_vector<First>;
