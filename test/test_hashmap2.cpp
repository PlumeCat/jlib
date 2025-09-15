// test_hashmap2.cpp
#include <unordered_map>

#include <jlib/test_framework.h>
#include <jlib/log.h>
#include <jlib/hash_map2.h>

using UM = std::unordered_map<std::string, std::string>;
using HM = hashmap<std::string, std::string>;

// random string generator
static auto S() {
    return std::string(10, char('A' + rand() % 26));
}

TEST("hashmap2 insert") {
    auto h = HM{};
    h.insert_or_assign("hello", "world");
    ASSERT(h.at("hello") == "world");
}

TEST("hashmap2 many insert, no overwrite") {
    auto h = HM{};
    for (auto i = 0; i < 100; i++) {
        auto s = S() + std::to_string(i);
        h.insert_or_assign(s, s);
        ASSERT(h.at(s) == s);
    }
}

TEST("hashmap2 many set (possible overwrites)") {
    auto h = HM {};
    for (auto i = 0; i < 100; i++) {
        auto s = S();
        h.insert_or_assign(s, s);
        ASSERT(h.at(s) == s);
    }
}

TEST("hashmap2 fuzzy test") {
    srand(time(nullptr));

    auto hm = HM {};
    auto um = UM {};

    // bunch of random actions
    for (auto i = 0; i < 1000; i++) {
        switch (rand() % 3) {
        case 0: {
                // add random element
                auto k = S() + std::to_string(i);
                auto v = S();
                um.insert_or_assign(k, v);
                hm.insert_or_assign(k, v);
            }
            break;
        case 1:
            // delete random element
            if (um.size() > 24) {
                auto n = rand() % um.size();
                auto e = um.begin(); for (auto i = 0; i < n; i++) e++;
                auto k = e->first;
                um.erase(k);
                hm.erase(k);
            }
            break;
        case 2:
            // update random element
            if (um.size()) {
                auto n = rand() % um.size();
                auto e = um.begin(); for (auto i = 0; i < n; i++) e++;
                auto k = e->first;
                auto v = S();
                um.insert_or_assign(k, v);
                hm.insert_or_assign(k, v);
            }
            break;
        }
    }

    // final check using unordered_map's iteration
    ASSERT(hm.size() == um.size());
    for (auto& [ k, v ] : um) {
        ASSERT(hm.at(k) == v);
    }
}
