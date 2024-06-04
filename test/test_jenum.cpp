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

TEST("jenum output") {
    log("TestEnum {\n\t",
        TestEnum::A, ",\n\t", TestEnum::B, ",\n\t", TestEnum::C, ",\n\t", TestEnum::D,
    "\n}");
}

TEST("jenum default value") {
    auto x = TestEnum {};
    auto y = 2;
    log((int)x, y);
}