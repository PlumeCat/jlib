#pragma once

#include <iostream>
#include <sstream>
#include <cstdint>
#include <thread>

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

template<bool Space, typename Arg> std::ostream& log_sp(std::ostream& o, Arg&& arg) {
    return log_stream(o, std::forward<Arg>(arg));
}
template<bool Space, typename First, typename... Args> std::ostream& log_sp(std::ostream& o, First&& first, Args&&... args) {
    log_stream(o, first);

    using first_t = std::decay_t<First>;
    if constexpr (Space
        && !std::is_same_v<first_t, Colors::Codes>
        && !std::is_same_v<first_t, decltype(std::hex)>
    ) {
        o << ' ';
    }
    return log_sp<Space>(o, std::forward<Args>(args)...);
}

template<bool Space, bool Prefix, typename... Args> std::ostream& log_pf(std::ostream& o, Args&&... args) {
    if constexpr (Prefix) {
        return log_sp<Space>(o, "LOG:", std::forward<Args>(args)..., '\n');
    } else {
        return log_sp<Space>(o, std::forward<Args>(args)..., '\n');
    }
}

template<bool Space=true, bool Prefix=true, typename...Args> void log(Args&&... args) {
    auto s = std::ostringstream {};
    log_pf<Space, Prefix>(s, std::forward<Args>(args)...);
    const auto str = s.str();
    std::cerr << str;
#ifdef JLIB_LOG_VISUALSTUDIO
    OutputDebugString(str.c_str());
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
