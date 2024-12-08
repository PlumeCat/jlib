#include <jlib/jenum.h>
#include <jlib/test_framework.h>


jenum_class(TestJenum,
    A = 1,
    B,
    C = B == 4 ? 5 : 6,
    D,
    E = (int)TestJenum::C + 5
);

TEST("compile time checks") {
    // jenum_internal::compile_time_checks();
    log<false>("comptime checks:", jenum_internal::templated_func_name<int>());
}

TEST("jenum") {
    ASSERT((int)TestJenum::A == 1);
    ASSERT((int)TestJenum::B == 2);
    ASSERT((int)TestJenum::C == 6);
    ASSERT((int)TestJenum::D == 7);
    ASSERT((int)TestJenum::E == 11);
}

TEST("jenum output") {
    log("TestEnum {\n\t",
        TestJenum::A, ",\n\t", TestJenum::B, ",\n\t", TestJenum::C, ",\n\t", TestJenum::D,
    "\n}");
}

TEST("jenum default value") {
    auto x = TestJenum {};
    auto y = 2;
    log((int)x, y);
}