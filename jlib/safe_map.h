#pragma once

#include <unordered_map>

template<typename K, typename V, typename Hash = std::hash<K>, typename Cmp = std::equal_to<K>>
class safe_map final : private std::unordered_map<K, V, Hash, Cmp> {
    using base = std::unordered_map<K, V, Hash, Cmp>;

public:
    using base::base;
    using base::value_type;
    
    using base::at;
    using base::try_emplace;
    using base::insert_or_assign;

    using base::size;
    using base::clear;

    using base::find;
    using base::begin;
    using base::end;
    using base::cbegin;
    using base::cend;
};