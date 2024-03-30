#include <jlib/test_framework.h>

TEST("terminal colours") {
    log(Colors::FG_BLACK, "FG_BLACK");
    log(Colors::FG_RED, "FG_RED");
    log(Colors::FG_GREEN, "FG_GREEN");
    log(Colors::FG_YELLOW, "FG_YELLOW");
    log(Colors::FG_BLUE, "FG_BLUE");
    log(Colors::FG_MAGENTA, "FG_MAGENTA");
    log(Colors::FG_CYAN, "FG_CYAN");
    log(Colors::FG_WHITE, "FG_WHITE");
    log(Colors::FG_DEFAULT, "FG_DEFAULT");

    log(Colors::FG_BLACK2, "FG_BLACK2");
    log(Colors::FG_RED2, "FG_RED2");
    log(Colors::FG_GREEN2, "FG_GREEN2");
    log(Colors::FG_YELLOW2, "FG_YELLOW2");
    log(Colors::FG_BLUE2, "FG_BLUE2");
    log(Colors::FG_MAGENTA2, "FG_MAGENTA2");
    log(Colors::FG_CYAN2, "FG_CYAN2");
    log(Colors::FG_WHITE2, "FG_WHITE2");

    log(Colors::BG_BLACK, "BG_BLACK", Colors::BG_DEFAULT);
    log(Colors::BG_RED, "BG_RED", Colors::BG_DEFAULT);
    log(Colors::BG_GREEN, "BG_GREEN", Colors::BG_DEFAULT);
    log(Colors::BG_YELLOW, "BG_YELLOW", Colors::BG_DEFAULT);
    log(Colors::BG_BLUE, "BG_BLUE", Colors::BG_DEFAULT);
    log(Colors::BG_MAGENTA, "BG_MAGENTA", Colors::BG_DEFAULT);
    log(Colors::BG_CYAN, "BG_CYAN", Colors::BG_DEFAULT);
    log(Colors::BG_WHITE, "BG_WHITE", Colors::BG_DEFAULT);
    log(Colors::BG_DEFAULT, "BG_DEFAULT", Colors::BG_DEFAULT);
}