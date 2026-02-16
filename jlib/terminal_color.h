#pragma once

#include <ostream>
#include <string>

class Colors {
public:
    enum Codes : uint32_t {
        RESET = 0,

        FG_BLACK = 30,
        FG_RED = 31,
        FG_GREEN = 32,
        FG_YELLOW = 33,
        FG_BLUE = 34,
        FG_MAGENTA = 35,
        FG_CYAN = 36,
        FG_WHITE = 37,
        FG_DEFAULT = 39,

        FG_BLACK2 = 90,
        FG_RED2 = 91,
        FG_GREEN2 = 92,
        FG_YELLOW2 = 93,
        FG_BLUE2 = 94,
        FG_MAGENTA2 = 95,
        FG_CYAN2 = 96,
        FG_WHITE2 = 97,

        BG_BLACK = 40,
        BG_RED = 41,
        BG_GREEN = 42,
        BG_YELLOW = 43,
        BG_BLUE = 44,
        BG_MAGENTA = 45,
        BG_CYAN = 46,
        BG_WHITE = 47,
        BG_DEFAULT = 49,
    };
};

inline std::string operator+(const std::string& s, Colors::Codes c) {
    return s + "\033[" + std::to_string(int(c)) + 'm';
}
inline std::string operator+(Colors::Codes c, const std::string& s) {
    return "\033[" + std::to_string(int(c)) + 'm' + s;
}

inline std::ostream& operator<<(std::ostream& o, Colors::Codes c) {
    return o << "\033[" << int(c) << 'm';
}
inline std::string operator+(const std::string& s, Colors::Codes c) {
    return s + "\033[" + std::to_string(int(c)) + 'm';
}
inline std::string operator+(Colors::Codes c, const std::string& s) {
    return "\033[" + std::to_string(int(c)) + 'm' + s;
}