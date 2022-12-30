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

TEST("test static heapsort") {
    constexpr auto N = 1000;
    auto input = array<uint32_t, N>{};
    auto output = array<uint32_t, N> { { 0 } };

    // random inputs
    srand(time(nullptr));
    for (auto& i: input) {
        i = rand() % 100;
    }
    
    heapsort(input, output);

    // make sure output is in order
    for (auto i = 1; i < N; i++) {
        ASSERT(output[i-1] >= output[i]);
    }
}

TEST("pop empty heap") {
    auto h = dynamic_heap<int>{};
    ASSERT_THROWS(h.pop());
}

TEST("test dynamic heapsort") {
    constexpr auto N = 10000;
    auto input = vector<uint32_t>();
    auto output = vector<uint32_t>();
    
    srand(time(nullptr));
    for (auto i = 0; i < N; i++) {
        input.emplace_back(rand() % N);
    }

    auto before = chrono::steady_clock::now();
    heapsort(input, output);
    auto after = chrono::steady_clock::now();
    log("sorting took",
        chrono::duration_cast<chrono::microseconds>(after - before).count() / 1000.f,
        "ms"
    );

    for (auto i = 1; i < N; i++) {
        ASSERT(output[i] <= output[i-1]);
    }
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
