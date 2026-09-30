#pragma once

/*
test framework
only for super simple basic sanity checks (inline)
you should use more advanced gtest for more serious stuff

usage:

    - define tests with the TEST() macro, see below
    - tests can be defined in any cpp
        avoid defining in headers (probably breaks with multiply-defined symbols if a header is included in multiple TU)
        can be inline next to the definition of whatever is being tested
        or in separate dedicated 'test-*.cpp' files
    - call run_tests() somewhere, preferably at the top of main(), see below
    - build and run with the macro ENABLE_TEST defined
        it will run all tests instead of the program
        you can run tests in debug and release mode

example:

    // in "some_file.cpp"

    TEST("some test") {
        auto my_class = MyClass();
        my_class.set_foo(true);
        assert(my_class.get_foo());
    }
    TEST("another test") {
        auto x = 1;
        auto y = 2;
        if (x == y) {
            FAIL("detailed message")
        }
    }


    // in "main.cpp"
    int main() {
        run_tests()
        ...
    }

*/

#include "log.h"
#include "terminal_color.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

// util
#ifndef paste
#    define paste(a, b) a##b
#endif

// asserts
#define ASSERT(...)                                                  \
    if (!(__VA_ARGS__)) {                                            \
        throw std::runtime_error("Assertion failed: " #__VA_ARGS__); \
    }
#define ASSERT_THROWS(...)                                      \
    do {                                                        \
        try {                                                   \
            __VA_ARGS__;                                        \
        } catch (...) {                                         \
            break;                                              \
        }                                                       \
        throw std::runtime_error("Should have thrown, didn't"); \
    } while (0);

#ifdef ENABLE_TEST
struct TestBase;

std::vector<TestBase*>& ALL_TESTS();

struct TestBase {
    const char* msg;
    const char* file;
    int line;

    inline TestBase(const char* msg, const char* file, int line):
        msg(msg),
        file(file),
        line(line) {
        ALL_TESTS().push_back(this);
    }

    inline bool operator()() {
        try {
            func();
            report(true, nullptr);
            return true;
        } catch (std::exception& e) {
            report(false, e.what());
            return false;
        }
    }

    inline void report(bool success, const char* error) {
        // Format like GCC error messages so the visual studio code build configuration can click through to failing tests
        auto color = success ? Colors::FG_GREEN : Colors::FG_RED;
        auto message = success ? " success: " : " error: (test failed) ";

        // if running in VScode, do special formatting so it looks like GCC output
        // this allows the user to double click a failing test to go to the source
        if (auto env = std::getenv("TERM_PROGRAM"); env && env == std::string { "vscode" }) {
            log<false, false>(file, ":", line, color, message, Colors::FG_DEFAULT, msg, error ? ": " : "", error ? error : "");
        } else {
            auto filename = std::filesystem::path(file).filename().string();
            log<false, false>(
                Colors::FG_CYAN2, filename, ":", line, color, message, Colors::FG_DEFAULT, msg, error ? ": " : "", error ? error : ""
            );
        }
    }

    virtual void func() = 0;
};

// template<size_t N> struct StringLiteral {
//     constexpr StringLiteral(const char (&str)[N]) {
//         std::copy_n(str, N, value);
//     }
//     char value[N];
// };
template <size_t Counter, size_t Line>
struct Test final: public TestBase {
    Test(const char* file, const char* msg):
        TestBase(msg, file, Line) {}

    virtual void func() override;
};

constexpr inline uint32_t comptime_hash(const char* s) {
    auto hash = 5381u;
    while (auto c = *s++) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

#    define TEST_(file, line, counter, ...)                                           \
        static auto paste(__test_, counter) = Test<counter + comptime_hash(file), line>(file, __VA_ARGS__); \
        template <>                                                                   \
        void Test<counter + comptime_hash(file), line>::func()
#    define TEST(...) TEST_(__FILE__, __LINE__, __COUNTER__, __VA_ARGS__)
#    define CHECK(...)                                                                \
        TEST_(__FILE__, __LINE__, __COUNTER__, #__VA_ARGS__) { ASSERT(__VA_ARGS__); }

#else
#    define TEST(...)
#    define CHECK(...)
#endif

#ifdef JLIB_TEST_IMPLEMENTATION

std::vector<TestBase*>& ALL_TESTS() {
    static auto all_tests = std::vector<TestBase*> {};
    return all_tests;
}

int run_tests(const std::vector<std::string_view>& subset) {
    log(Colors::FG_YELLOW2, "Running tests", Colors::FG_DEFAULT);
    auto passed = 0;

    // if a subset is passed, remove tests not in the subset
    if (subset.size()) {
        std::erase_if(ALL_TESTS(), [&](auto& t) {
            auto stem = std::filesystem::path { t->file }.stem().string();
            return std::find(subset.begin(), subset.end(), std::string_view { stem }) == subset.end();
        });
    }

    for (auto& t : ALL_TESTS()) {
        passed += (*t)() ? 1 : 0;
        // log("test: ", t->file, t->line, t->msg);
        // log("test", std::filesystem::path { t->file }.stem(), t->line, t->msg);
    }
    auto failed = ALL_TESTS().size() - passed;
    log<false>(Colors::FG_YELLOW2, "Tests complete", Colors::FG_DEFAULT, ": ", passed, " passed, ", failed, " failed");

    return failed == 0;
}
#endif
