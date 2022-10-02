#pragma once


/*
test framework
only for super simple basic sanity checks (inline)
you should use more advanced gtest for more serious stuff

usage:

    - define tests with the TEST() macro, see below
    - tests can be defined in any cpp, avoid defining in headers (probably breaks with multiply-defined symbols if a header is included in multiple TU)
        can be inline next to the definition of whatever is being tested
        or in separate dedicated 'test-*.cpp' files
        avoid tests in header files
    - put IMPLEMENT_TESTS() somewhere once,
        in a translation unit that includes test_framework.h;
        can be its own file or just above main
    - call RUN_TESTS() somewhere, preferably at the top of main(), see below
    - build and run with the macro ENABLE_TEST defined
        it will run all tests instead of the program
        you can run tests in debug and release mode
    
    Example:

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

*/

#include <iostream>
#include <vector>

#include <jlib/terminal_color.h>
#include <jlib/log.h>

#ifdef ENABLE_TEST
struct TestBase;

std::vector<TestBase*>& ALL_TESTS();
int& ALL_TESTS_RESULT();

struct TestBase {
    const char* msg;
    const char* file;
    int line;

    inline TestBase(const char* msg, const char* file, int line):
        msg(msg), file(file), line(line) {
        ALL_TESTS().push_back(this);
    }
    inline void operator()() {
        try {
            log("Running test: ", msg);
            func();
            log("...Done");
            report(true, "");
        } catch (std::exception& e) {
            report(false, e.what());
        }
    }

    inline void report(bool success, const char* error) {
        // Format like GCC error messages so the visual studio code build configuration can click through to failing tests
        auto color = success ? Colors::FG_GREEN : Colors::FG_RED;
        auto message = success ? " success: " : " error: (test failed) ";

        log<false, false>(file, ":", line, ":5:", color, message, Colors::FG_RED, Colors::FG_DEFAULT, msg, error ? ": " : "", error);
    }

    virtual void func() = 0;
};

// template<const char* File, size_t Line>
// struct Test {

// };

template<size_t N>
struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    char value[N];
};

template<StringLiteral Filename, size_t Line>
struct Test final : public TestBase {
    Test(const char* msg): TestBase(msg, Filename.value, Line) {}
    virtual void func() override;
};


#define paste(a, b) a##b

#define TEST_(test_msg, file, line, counter)\
    static auto paste(__test_, counter) = Test<file, line>(test_msg);\
    template<> void Test<file, line>::func()

#define TEST(test_msg) TEST_(test_msg, __FILE__, __LINE__, __COUNTER__)

#define ASSERT(cond) if (!(cond)) { throw std::runtime_error("Assertion failed: " #cond); }
#define ASSERT_THROWS(code) \
    do {\
        try { code; }\
        catch (...) { break; }\
        throw std::runtime_error("Should have thrown, didn't");\
    } while (0);\

#define IMPLEMENT_TESTS()\
    std::vector<TestBase*>& ALL_TESTS() {\
        static auto all_tests = std::vector<TestBase*>{};\
        return all_tests;\
    }\
    int& ALL_TESTS_RESULT() {\
        static auto result = 0;\
        return result;\
    }\
    void run_tests() {\
        log(Colors::FG_YELLOW, "Running tests", Colors::FG_DEFAULT);\
        for (auto& t: ALL_TESTS()) {\
            (*t)();\
        }\
        log(Colors::FG_YELLOW, "Tests complete", Colors::FG_DEFAULT);\
    }

#define RUN_TESTS() { run_tests(); return ALL_TESTS_RESULT();}


#else
#define TEST_(msg, line) void paste(test_unused_, line)
#define TEST(msg) TEST_(msg, __LINE__)
#define IMPLEMENT_TESTS()
#define RUN_TESTS()
#endif
