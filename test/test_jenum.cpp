#include <jlib/jenum.h>
#include <jlib/test_framework.h>

template<typename E> constexpr int if_equal(E e, int test, int yes, int no) {
    return (e == test) ? yes : no;
}

jenum_class(TestEnum,
    A = 1,
    B,
    C = if_equal(B, 4, 5, 6),
    D = 3,
    E = (int)TestEnum::C + 5
)

// void jenum_parse_strip(std::string_view& str) {
//     while (str.size() && std::isspace(str[0])) {
//         str.remove_prefix(1);
//     }
// }

void jenum_parse(const char *enum_def) {
    log("PARSE ENUM", enum_def);

    /*

    Enum syntax for enumerator list:

        identifier,
        identifier = constant-expression,

    */

    // auto str = std::string_view { enum_def };
    // while (str.size()) {
    //     jenum_parse_strip(str);
    // }
}

TEST("jenum output") {
    log<false>("TestEnum {\n\t",
        TestEnum::A, ",\n\t", TestEnum::B, ",\n\t", TestEnum::C, ",\n\t", TestEnum::D,
    "\n}");
}

TEST("jenum default value") {
    auto x = TestEnum {};
    auto y = 2;
    log((int)x, y);
}