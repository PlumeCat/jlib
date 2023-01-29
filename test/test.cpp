#define TESTS_IMPLEMENTATION
#define JLIB_IMPLEMENTATION
#include <jlib/test_framework.h>
#include <jlib/generic_ostream.h>
#include <jlib/log.h>
#include <jlib/heapsort.h>
#include <jlib/text_file.h>

#include <iostream>
#include <array>
#include <sstream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <filesystem>
using namespace std;

TEST("test pwd") {
    log("CWD: ", filesystem::current_path());
}

TEST("test file read") {
    ASSERT(read_text_file("test/test_data.txt").value() == 
        "hello world\n"
        "this is a c++ program\n"
        "end\n"
    );
}

TEST("generic ostream") {
    auto v = vector<int>{ 1, 2, 3, 4, 5 };
    auto p = map<string, int>{};
    p["one"] = 1;
    p["two"] = 2;
    p["three"] = 3;

    auto s = stringstream{};
    s << v << '|';
    s << p;

    ASSERT(s.str() == 
        "{ 1, 2, 3, 4, 5 }|{ [ one, 1 ], [ three, 3 ], [ two, 2 ] }"
    );
}

IMPLEMENT_TESTS()
int main(int argc, char* argv[]) {
    RUN_TESTS()
    return 0;
}
