#pragma once

#include <ostream>
#include <sstream>
#include <jlib/generic_ostream.h>
#include <jlib/terminal_color.h>

// very simple thread safe logging utility

#ifdef JLIB_LOG_VISUALSTUDIO
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

template<bool Space, typename Arg>
std::ostream& log_stream(std::ostream& o, const Arg& arg) {
    return o << arg;
}

template<bool Space, typename First, typename ...Args>
std::ostream& log_stream(std::ostream& o, const First& first, const Args&... args) {
    log_stream<Space>(o, first);
    if constexpr(Space && !std::is_same_v<First, Colors::Codes>) {
        o << ' ';
    }
    log_stream<Space>(o, args...);
    return o;
}

template<bool Space = true, bool Prefix = true, typename ...Args>
void log(const Args&... args) {
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
