#include <jlib/test_framework.h>

#include <type_traits>

using namespace std;

TEST("is_base_of") {

    struct X {
        mutable int a;
    };

    const X x = { .a = 100 };
    x.a = 1;


}

struct X{};

// take a copy of x
void f(X x) {

}

// move from x - take ownership of X's resources
void f(X&& x) {

}

void f(X* x) {}
void f(const X* x) {}
void f(const unique_ptr<X>& x) {}