#pragma once

#include <vector>

template<typename T> class safe_vector final : private std::vector<T> {
    using base = std::vector<T>;

public:
    using base::base;
    using base::value_type;

    using base::clear;
    using base::resize;
    using base::reserve;
    
    using base::size;

    using base::emplace_back;
    using base::push_back;

    using base::at;
    using base::data;
    using base::begin;
    using base::cbegin;
    using base::end;
    using base::cend;
};