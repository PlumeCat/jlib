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
    - put IMPLEMENT_TESTS() somewhere once,
        in a translation unit that includes test_framework.h;
        can be its own file or just above main
    - call RUN_TESTS() somewhere, preferably at the top of main(), see below
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

    IMPLEMENT_TESTS()
    int main() {
        RUN_TESTS()
        ...
    }

tips:
    put IMPLEMENT_TESTS() and RUN_TESTS() in a different file
    and build that file separately as a different test artifact
    - the tests and test data won't get linked into your main build

*/

#include <filesystem>
#include <iostream>
#include <vector>

#include "log.h"
#include "terminal_color.h"

// util
#ifndef paste
#define paste(a, b) a##b
#endif

// asserts
#define ASSERT(...)                                                                                                                   \
    if (!(__VA_ARGS__)) {                                                                                                             \
        throw std::runtime_error("Assertion failed: " #__VA_ARGS__);                                                                  \
    }
#define ASSERT_THROWS(...)                                                                                                            \
    do {                                                                                                                              \
        try {                                                                                                                         \
            __VA_ARGS__;                                                                                                              \
        } catch (...) {                                                                                                               \
            break;                                                                                                                    \
        }                                                                                                                             \
        throw std::runtime_error("Should have thrown, didn't");                                                                       \
    } while (0);

#ifdef ENABLE_TEST
struct TestBase;

std::vector<TestBase*>& ALL_TESTS();
int& ALL_TESTS_RESULT();
bool& PRIORITY();

struct TestBase {
    const char* msg;
    const char* file;
    int line;

    inline TestBase(const char* msg, const char* file, int line, bool priority):
        msg(msg),
        file(file),
        line(line) {
        if (PRIORITY()) {
            return;
        }
        if (priority) {
            ALL_TESTS().clear();
            PRIORITY() = true;
        }
        ALL_TESTS().push_back(this);
    }
    inline void operator()() {
        try {
            // log("Running test: ", msg);
            func();
            // log("...Done");
            report(true, nullptr);
        } catch (std::exception& e) {
            report(false, e.what());
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

template<size_t N> struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    char value[N];
};

template<StringLiteral Filename, size_t Line> struct Test final : public TestBase {
    Test(const char* msg, bool just_me = false):
        TestBase(msg, Filename.value, Line, just_me) {}
    virtual void func() override;
};

#define TEST_(file, line, counter, ...)                                                                                               \
    static auto paste(__test_, counter) = Test<file, line>(__VA_ARGS__);                                                              \
    template<> void Test<file, line>::func()

#define TEST(...) TEST_(__FILE__, __LINE__, __COUNTER__, __VA_ARGS__)

#define CHECK(...) TEST_(__FILE__, __LINE__, __COUNTER__, #__VA_ARGS__) { ASSERT(__VA_ARGS__); }

#define IMPLEMENT_TESTS()                                                                                                             \
    std::vector<TestBase*>& ALL_TESTS() {                                                                                             \
        static auto all_tests = std::vector<TestBase*> {};                                                                            \
        return all_tests;                                                                                                             \
    }                                                                                                                                 \
    int& ALL_TESTS_RESULT() {                                                                                                         \
        static auto result = 0;                                                                                                       \
        return result;                                                                                                                \
    }                                                                                                                                 \
    bool& PRIORITY() {                                                                                                                \
        static auto prio = false;                                                                                                     \
        return prio;                                                                                                                  \
    }                                                                                                                                 \
    void run_tests() {                                                                                                                \
        log(Colors::FG_YELLOW2, "Running tests", Colors::FG_DEFAULT);                                                                 \
        for (auto& t : ALL_TESTS()) {                                                                                                 \
            (*t)();                                                                                                                   \
        }                                                                                                                             \
        log(Colors::FG_YELLOW2, "Tests complete", Colors::FG_DEFAULT);                                                                \
    }

#define RUN_TESTS()                                                                                                                   \
    {                                                                                                                                 \
        run_tests();                                                                                                                  \
        return ALL_TESTS_RESULT();                                                                                                    \
    }

#else
#define TEST_(msg, file, line, counter) static void paste(test_unused_, line)()
#define TEST(msg) TEST_(msg, __FILE__, __LINE__, __COUNTER__)
#define CHECK(cond)

#define IMPLEMENT_TESTS()
#define RUN_TESTS()
#endif
