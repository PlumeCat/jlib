#include <jlib/jenum.h>
#include <jlib/test_framework.h>

jenum_class(T, A = 1, B, C = B == 4 ? 5 : 6, D, E = (int)T::C + 5);

TEST("jenum") {
    ASSERT((int)T::A == 1);
    ASSERT((int)T::B == 2);
    ASSERT((int)T::C == 6);
    ASSERT((int)T::D == 7);
    ASSERT((int)T::E == 11);
}

TEST("jenum value assignment") {}
TEST("jenum to string") {}
TEST("jenum from string") {}
TEST("jenum from integer") {}
