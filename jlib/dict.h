// dict.h

#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#pragma once

template<typename ... Bases> struct overload_call_op : Bases ... {
    using is_transparent = void;
    using Bases::operator() ... ;
};
struct char_pointer_hash {
    auto operator()( const char* ptr ) const noexcept {
        return std::hash<std::string_view>{}(ptr);
    }
};
using hash_string = overload_call_op<
    std::hash<std::string>,
    std::hash<std::string_view>,
    char_pointer_hash
>;
using string_set = std::unordered_set<std::string, hash_string, std::equal_to<>>;
template<typename T> using dict = std::unordered_map<std::string, T, hash_string, std::equal_to<>>;
// TODO: requires hash(string) === hash(string_view) for the same characters
// need to ensure this is always true (always true for MSVC)
