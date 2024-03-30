#include <jlib/generic_ostream.h>
#include <jlib/log.h>
#include <jlib/test_framework.h>

#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>

TEST("generic ostream maps and sets") {
    auto s = std::map<std::string, int> {
        { "one", 1 },
        { "two", 2 },
        { "three", 3 }
    };
    auto t = std::unordered_map<std::string, int> { s.begin(), s.end() };
    auto u = std::set<std::string> {};
    auto v = std::unordered_set<std::string> {};
    for (auto& [ k, _ ] : s) {
        u.emplace(k);
        v.emplace(k);
    }

    log("maps:", s, t);
    log("sets: ", u, v);
}

// TEST("generic ostream tuples and pairs") {
//     log("simple pair: ", std::pair { 1, 2.f });
//     log("simple pair: ", std::pair { 1, "two" });

//     auto t = std::tuple { 1, "two", 3.f };
//     log("simple tuple: ", t);
//     log("tuple of tuple: ", std::tuple { std::tuple { 1, "two", 3.f }, std::tuple { 4, "five", 6.f } });
//     log("deranged tuple: ", std::tuple { 1, "two", 3.f, std::array { 1, 2, 3 }, std::array { std::vector<float> { 1, 2, 3, 4.0 } } });
// }

#include <array>
#include <list>
#include <vector>
TEST("generic ostream list/array/vector") {
    log(std::list { 1, 2, 3, 4, 5 });
    log(std::array { 1, 2, 3, 4, 5 });
    log(std::vector { 1, 2, 3, 4, 5 });
}

#include <deque>
#include <stack>
TEST("generic ostream deque") {
    log(std::deque { 1, 2, 3, 4, 5 });
}

// TODO: maybe try and support tuple, optional, any, variant, queue, stack

// TODO: jlib dag, heap, stack_stack, hash_map, swiss_vector