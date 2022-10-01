#pragma once

#include <ostream>
#include <typeinfo>
#include <tuple>

// sfinae helpers
#define DisableTemplate(candidate, disabled) \
    typename = std::enable_if_t<!std::is_same_v<candidate, disabled>>

// helper for pair<>
template<typename L, typename R>
std::ostream& operator << (std::ostream& o, const std::pair<L, R>& pair) {
    return o << "[ " << pair.first << ", " << pair.second << " ]";
}

// helper for tuple
template<typename ...Args>
std::ostream& operator<< (std::ostream& o, const std::tuple<Args...>& t){
    o << '(';
    // std::tuple<Args...>::
    return o << ')';
}


// you can output so many different types with this bad boy *slaps signature*
template<template<typename...> typename Container, typename ...Args,
    DisableTemplate(Container<Args...>, std::string),
    DisableTemplate(Container<Args...>, std::stringstream)>
std::ostream& operator << (std::ostream& o, const Container<Args...>& c) {
    o << '{';
    if (c.size()) {
        o << ' ' << *(c.begin());
        auto i = c.begin();
        i++;
        for (; i != c.end(); i++) {
            o << ", " << *i;
        }
    }
    return o << " }";
}
