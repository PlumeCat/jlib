#include <jlib/jenum.h>
#include <jlib/test_framework.h>

jenum_class(TestJenum, X, A = 1, B, C = B == 4 ? 5 : 6, D = std::tuple_size_v<std::tuple<int, int, int, int, int, int, int>>, E = (int)TestJenum::C + 5);

jenum(T0, a, b, c);
jenum_base(T1, char, d, e, f);
jenum_class(T2, a, b, c);
jenum_class_base(T3, uint32_t, a, b, c);
// TEST("underlying types") {
//     // underlying type is impl defined if not provided
//     static_assert(std::is_same_v<std::underlying_type_t<T1>, char>);
//     static_assert(std::is_same_v<std::underlying_type_t<T3>, uint32_t>);
// }

// TEST("jenum helper") {
//     const auto h =  jenum_s<TestJenum> {};

//     ASSERT(h.X == 0);
//     ASSERT(h.A == 1);
//     ASSERT(h.B == 2);
//     ASSERT(h.C == 6);
//     ASSERT(h.D == 7);
//     ASSERT(h.E == 11);

//     ASSERT(h[0] == 0);
//     ASSERT(h[1] == 1);
//     ASSERT(h[2] == 2);
//     ASSERT(h[3] == 6);
//     ASSERT(h[4] == 7);
//     ASSERT(h[5] == 11);
// }

// TEST("jenum") {
//     ASSERT((int)TestJenum::A == 1);
//     ASSERT((int)TestJenum::B == 2);
//     ASSERT((int)TestJenum::C == 6);
//     ASSERT((int)TestJenum::D == 7);
//     ASSERT((int)TestJenum::E == 11);

//     ASSERT(jenum_s<TestJenum> {}.X == 0);
//     ASSERT(jenum_s<TestJenum> {}.count() == 6);

//     log("string: ", jenum_string<TestJenum>(), "/string");
// }

// TEST("jenum output") {
//     auto x = std::stringstream {};
//     x << TestJenum::X << "|" << TestJenum::E;
//     log(x.str());
//     ASSERT(x.str() == "TestJenum::X|TestJenum::E");
// }

// TEST("jenum default value") {
//     auto x = TestJenum {};
//     ASSERT(x == TestJenum::X);
//     ASSERT((int)x == 0);
// }
