#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>

#include "generic_ostream.h"
#include "terminal_color.h"

// very simple thread safe logging utility

#if (defined JLIB_LOG_VISUALSTUDIO && defined _MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#define OutputDebugString(x)
#endif

struct log_pad {
    size_t n = 0;
    char ch = ' ';
};

#define QUOTE quote {} |
template<char Q = '\''> struct quote {};
template<typename T, char Q> struct quotewrap { const T& t; };
template<typename T, char Q='\''> quotewrap<T,Q> operator|(const quote<Q>& q, const T& t) {
    return quotewrap<T, Q> { t };
}
template<typename T, char Q> std::ostream& operator<<(std::ostream& o, const quotewrap<T, Q>& q) {
    return o << Q << q.t << Q;
}


std::ostream& operator<<(std::ostream& o, const uint8_t& arg);
std::ostream& operator<<(std::ostream& o, const log_pad& align);

std::ostream& log_stream(std::ostream& o, const std::string& arg);
std::ostream& log_stream(std::ostream& o, const std::string_view& arg);


template<typename Arg> std::ostream& log_stream(std::ostream& o, const Arg& arg) {
    return o << arg;
}

// helper for pair
template <typename L, typename R> std::ostream& log_stream(std::ostream& o, const std::pair<L,R>& pair) {
    // return o << "[ " << pair.first << ", " << pair.second << " ]";
    o << "[ ";
    log_stream(o, pair.first);
    o << ", ";
    log_stream(o, pair.second);
    return o << " ]";
}

// helpers for tuple
template <size_t N, typename... Args> std::ostream& log_tuple_helper(std::ostream& o, const std::tuple<Args...>& t) {
    log_stream(o, std::get<N>(t));
    if constexpr (N + 1 < std::tuple_size<std::decay_t<decltype(t)>>::value) {
        o << ", ";
        log_tuple_helper<N+1>(o, t);
    }
    return o;
}
template <typename... Args> std::ostream& log_stream(std::ostream& o, const std::tuple<Args...>& t) {
    o << '(';
    return log_tuple_helper<0>(o, t) << ')';
}

template<Container Arg> std::ostream& log_stream(std::ostream& o, const Arg& arg) {
    // return o << "CONTAINER:" << __FUNCTION__;
    o << '{';
    auto i = arg.cbegin();
    const auto e = arg.cend();
    if (i != e) {
        o << ' ';
        log_stream(o, *i);
        for (i++; i != e; i++) {
            o << ", "; log_stream(o, *i);
        }
    }
    return o << " }";
}


template<bool Space, typename Arg> std::ostream& log_sp(std::ostream& o, Arg arg) {
    return log_stream(o, arg);
}
template<bool Space, typename First, typename... Args> std::ostream& log_sp(std::ostream& o, First first, Args... args) {
    log_stream(o, first);
    if constexpr (Space && !std::is_same_v<First, Colors::Codes>) {
        o << ' ';
    }
    return log_sp<Space>(o, args...);
}

template<bool Space, bool Prefix, typename... Args> std::ostream& log_pf(std::ostream& o, Args... args) {
    if constexpr (Prefix) {
        return log_sp<Space>(o, "LOG:", args..., '\n');
    } else {
        return log_sp<Space>(o, args..., '\n');
    }
}

template<bool Space=true, bool Prefix=false, typename...Args> void log(Args... args) {
#ifdef JLIB_LOG_VISUALSTUDIO
    auto s = std::ostringstream {};
    log_pf<Space, Prefix>(s, args...);
    const auto str = s.str();
    OutputDebugString(str.c_str());
    std::cerr << str;
#else
    log_pf<Space, Prefix>(std::cerr, args...);
#endif
}

#ifdef JLIB_IMPLEMENTATION
std::ostream& operator<<(std::ostream& o, const uint8_t& arg) {
    return o << (int)arg;
}
std::ostream& operator<<(std::ostream& o, const log_pad& align) {
    auto pos = o.tellp() - std::streampos { 0 };
    while (pos++ < (decltype(pos))align.n) {
        o << align.ch;
    }
    return o;
}
std::ostream& log_stream(std::ostream& o, const std::string& arg) {
    return o << arg;
}
std::ostream& log_stream(std::ostream& o, const std::string_view& arg) {
    return o << arg;
}
#endif
