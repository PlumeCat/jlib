#include <jlib/jenum.h>
#include <jlib/test_framework.h>

jenum_class(T, X, A = 1, B, C = B == 4 ? 5 : 6, D, E = (int)T::C + 5);

TEST("jenum") {
    ASSERT((int)T::A == 1);
    ASSERT((int)T::B == 2);
    ASSERT((int)T::C == 6);
    ASSERT((int)T::D == 7);
    ASSERT((int)T::E == 11);
}

TEST("jenum output") {
    auto x = std::stringstream {};
    x << T::X << "|" << T::E;
    log(x.str());
    ASSERT(x.str() == "T::X|T::E");
}

TEST("jenum default value") {
    auto x = T {};
    auto y = 2;
    log((int)x, y);
}
