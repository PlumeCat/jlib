#pragma once

#include <ostream>
#include <string>
#include <tuple>

template <typename T> concept Container = requires(T t) {
    { t.cbegin() } -> std::same_as<typename T::const_iterator>;
    { t.cend() } -> std::same_as<typename T::const_iterator>;
};

template<typename T> concept NotString = (
    not (std::same_as<std::remove_cvref_t<T>, std::string> || std::same_as<std::remove_cvref_t<T>, std::string_view>)
);

template<typename T> concept NotStringContainer = (NotString<T> && Container<T>);

// helper for pair<L, R> for std::*_map types
template <typename L, typename R> std::ostream& operator<<(std::ostream& o, const std::pair<L, R>& pair) {
    return o << "[ " << pair.first << ", " << pair.second << " ]";
}

// helpers for tuple
template <size_t N, typename... Args> std::ostream& operator<<(std::ostream& o, const std::tuple<Args...>& t) {
    o << std::get<N>(t);
    if constexpr (N + 1 < std::tuple_size<std::decay_t<decltype(t)>>::value) {
        o << ", ";
        ::operator<< <0>(o, t);
    }
    return o;
}
template <typename... Args> std::ostream& operator<<(std::ostream& o, const std::tuple<Args...>& t) {
    o << '(';
    ::operator<< <0>(o, t);
    return o << ')';
}

template<NotStringContainer T> std::ostream& operator<<(std::ostream& o, T& c) {
    // TODO: make it work for raw arrays?
    o << '{';
    auto i = c.cbegin();
    const auto e = c.cend();
    if (i != e) {
        o << ' ' << *i;
        for (i++; i != e; i++) { o << ", " << *i; }
    }
    return o << " }";
}
