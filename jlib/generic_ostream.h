#pragma once

#include <ostream>
#include <tuple>

template<typename T> concept OutputDefined = requires(std::ostream o, T t) { operator << (o, t); };
template<typename T> concept OutputNotDefined = !OutputDefined<T>;
template<typename T> concept ContainerOutputNotDefined = requires(T t) {
    { t.size() } -> std::same_as<size_t>;
    { t.cbegin() } -> std::same_as<typename T::const_iterator>;
    { t.cend() } -> std::same_as<typename T::const_iterator>;
} && OutputNotDefined<T>;

// helper for pair<L, R> for std::*_map types
template<typename L, typename R>
std::ostream& operator << (std::ostream& o, const std::pair<L, R>& pair) {
    return o << "[ " << pair.first << ", " << pair.second << " ]";
}

// helpers for tuple
template<size_t N, typename ...Args>
std::ostream& operator<<(std::ostream& o, const std::tuple<Args...>& t) {
    o << std::get<N>(t);
    if constexpr(N+1 < std::tuple_size<std::decay_t<decltype(t)>>::value) {
        o << ", ";
        ::operator<< <0>(o, t);
    }
    return o;
}
template<typename ...Args>
std::ostream& operator<< (std::ostream& o, const std::tuple<Args...>& t){
    o << '(';
    ::operator<< <0>(o, t);
    return o << ')';
}

// you can output so many different types with this bad boy *slaps signature*
std::ostream& operator << (std::ostream& o, ContainerOutputNotDefined auto c) {
    // TODO: make it work for raw arrays?
    o << '{';
    if (c.size()) {
        auto i = c.cbegin();
        const auto end = c.cend();
        o << ' ' << *i;
        for (i++; i != end; i++) {
            o << ", " << *i;
        }
    }
    return o << " }";
}
