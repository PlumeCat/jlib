#pragma once

#include <iostream>
#include <sstream>
#include "jlib/generic_ostream.h"
#include "jlib/terminal_color.h"

// very simple thread safe logging utility

#if (defined JLIB_LOG_VISUALSTUDIO && defined _MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#define OutputDebugString(x)
#endif

struct log_pad { size_t n = 0; char ch = ' '; };

std::ostream& operator<<(std::ostream& o, const uint8_t& arg);
std::ostream& operator<<(std::ostream& o, const log_pad& align);

template<bool Space, typename Arg>
std::ostream& log_stream(std::ostream& o, Arg arg) {
    return o << arg;
}
template<bool Space, typename First, typename ...Args>
std::ostream& log_stream(std::ostream& o, First first, Args... args) {
    log_stream<Space>(o, first);
    if constexpr(Space && !std::is_same_v<First, Colors::Codes>) {
        o << ' ';
    }
    log_stream<Space>(o, args...);
    return o;
}

template<bool Space = true, bool Prefix = false, typename ...Args>
void log(Args... args) {
    auto s = std::ostringstream{};
    if constexpr(Prefix) {
        log_stream<Space>(s, "LOG:", args..., '\n');
    } else {
        log_stream<Space>(s, args..., '\n');
    }
    auto str = s.str();
    #ifdef JLIB_LOG_VISUALSTUDIO
    OutputDebugString(str.c_str());
    #endif
    std::cerr << str;
}


#ifdef JLIB_IMPLEMENTATION
std::ostream& operator<<(std::ostream& o, const uint8_t& arg) {
    return o << (int)arg;
}
std::ostream& operator<<(std::ostream& o, const log_pad& align) {
    auto pos = o.tellp() - std::streampos { 0 };
    while (pos++ < align.n) {
        o << align.ch;
    }
    return o;
}
#endif