// swiss_vector.h
#pragma once

#include <vector>

template<typename T>
class swiss_vector {
    vector<bool> active;
    vector<T> node;
};
