#pragma once

#include <ostream>

class Colors {
public:
    enum Codes {
        FG_RED      = 31,
        FG_GREEN    = 32,
        FG_YELLOW   = 93,
        FG_BLUE     = 34,
        FG_DEFAULT  = 39,
        BG_RED      = 41,
        BG_GREEN    = 42,
        BG_BLUE     = 44,
        BG_DEFAULT  = 49
    };
};

inline std::ostream& operator<< (std::ostream& o, Colors::Codes c) {
    return o << "\033[" << std::to_string(int(c)) << 'm';
}
