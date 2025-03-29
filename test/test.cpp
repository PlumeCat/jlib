#define JLIB_IMPLEMENTATION
#define TESTS_IMPLEMENTATION

#include "jlib/all.h"

IMPLEMENT_TESTS()
int main(int argc, char* argv[]) {
    // if (auto x = [] RUN_TESTS("test_hashmap"); x());
    if (auto x = [] RUN_TESTS("test_hashmap_fuzzy"); x());
}