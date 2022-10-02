#include <jlib/hash_map.h>
#include <jlib/test_framework.h>

using namespace std;

const char* x = __FILE__;

void foo() {
    cout << "BASE: " << x << endl;
}

TEST("hashmap") {
    throw runtime_error("hello world");
}
