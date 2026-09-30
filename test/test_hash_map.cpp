// test_hash_map.cpp
#include <unordered_map>

#include <jlib/test_framework.h>
#include <jlib/log.h>
#include <jlib/hash_map.h>

using UM = std::unordered_map<std::string, std::string>;
using HM = hash_map<std::string, std::string>;

// random string generator
static auto S() {
    return std::string(10, char('A' + rand() % 26));
}

TEST("hash_map insert") {
    auto h = HM{};
    h.insert_or_assign("hello", "world");
    ASSERT(h.at("hello") == "world");
}

TEST("hash_map many insert, no overwrite") {
    auto h = HM{};
    for (auto i = 0u; i < 10000u; i++) {
        auto s = S() + std::to_string(i);
        h.insert_or_assign(s, s);
        ASSERT(h.at(s) == s);
    }
}

TEST("hash_map many set (possible overwrites)") {
    auto h = HM {};
    for (auto i = 0u; i < 10000u; i++) {
        auto s = S();
        h.insert_or_assign(s, s);
        ASSERT(h.at(s) == s);
    }
}
