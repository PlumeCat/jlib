// swiss_vector.h
#pragma once

#include <vector>

template<typename T>
class swiss_vector {
public:
	swiss_vector() = delete;
	swiss_vector(const swiss_vector&) = default;
	swiss_vector(swiss_vector&&) = default;
	swiss_vector& operator=(const swiss_vector&) = default;
	swiss_vector& operator=(swiss_vector&&) = default;

	swiss_vector(size_t capacity = 1024): storage(capacity), is_free(capacity, false) {}

private:
	std::vector<T> storage;
	std::vector<size_t> free_slots;
	std::vector<bool> is_free;
};
