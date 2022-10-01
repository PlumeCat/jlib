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

    TestBase(const char* msg, const char* file, int line):
        msg(msg), file(file), line(line) {
        ALL_TESTS().push_back(this);
    }
    void operator()() {
        try {
            log("Running test: ", msg);
            func();
            log("...Done");
            report(true);
        } catch (...) {
            report(false);
        }
    }

    void report(bool success) {
        // Format like GCC error messages so the visual studio code build configuration can click through to failing tests
        auto color = success ? Colors::FG_GREEN : Colors::FG_RED;
        auto message = success ? " success: " : " error: (test failed) ";

        log<false, false>(file, ":", line, ":5:", color, message, Colors::FG_DEFAULT, msg);
    }

    virtual void func() = 0;
};


#define paste(a, b) a##b
#define TEST_(test_msg, file, line, counter) \
    struct paste(Test_, counter) final: public TestBase {\
        paste(Test_, counter)(const char* msg, const char* filename, int linenumber):\
            TestBase(test_msg, filename, linenumber) {}\
        virtual void func() override;\
    };\
    auto paste(___test_, counter) = paste(Test_, counter)(test_msg, file, line);\
    void paste(Test_, counter)::func()

#define TEST(test_msg) TEST_(test_msg, __FILE__, __LINE__, __COUNTER__)

#define ASSERT(cond) if (!(cond)) { throw runtime_error("Assertion failed: " #cond); }

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
