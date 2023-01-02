// swiss_vector.h
#pragma once

#include <vector>
using namespace std;

template<typename T>
class swiss_vector {
public:
	swiss_vector() = delete;
	swiss_vector(const swiss_vector&) = default;
	swiss_vector(swiss_vector&&) = default;
	swiss_vector& operator=(const swiss_vector&) = default;
	swiss_vector& operator=(swiss_vector&&) = default;

	swiss_vector(size_t capacity) {
		storage = vector<T>
	}
	vector<bool> is_free;

private:
	vector<T> storage;
	vector<size_t> free_slots;
};
