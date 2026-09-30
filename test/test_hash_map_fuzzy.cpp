#include "jlib/hash_map.h"
#include "jlib/test_framework.h"
#include "jlib/timer.h"

#include <unordered_map>
#include <chrono>
#include <string>
#include <vector>

namespace stdc = std::chrono;

// random key generator
std::string random_ascii_string() {
    const auto len = 10 + rand() % 10;
    auto key = std::string(len, 'a');
    for (auto i = 0; i < len; i++) {
        key[i] = rand() % 127 + 32;
    }
    return key;
}
std::vector<std::string> random_keys(int n) {
    auto keys = std::vector<std::string> {};
    for (auto i = 0; i < n; i++) {
        keys.emplace_back(random_ascii_string());
    }
    return keys;
}

struct timed_timer {
    uint32_t duration = 0;
    std::string_view name;
    timed_timer(auto f, auto name): name(name) {
        auto before = stdc::steady_clock::now();
        f();
        auto after = stdc::steady_clock::now();
        duration = stdc::duration_cast<stdc::microseconds>(after - before).count();
    }
    ~timed_timer() {
        log(Colors::FG_YELLOW2, "timer:", name, Colors::FG_DEFAULT, duration);
    }
};
struct timed_dummy { std::string_view name; };
template <typename F>
auto operator<<(timed_dummy d, F f) { return timed_timer(f, d.name); }
#ifndef paste
#define paste(a, b) a##b
#endif
#define _timed(name, c) auto paste(_timed_timer, c) = timed_dummy { name } << [&]
#define timed(name) _timed(name, __COUNTER__)

static auto S() {
    return std::string(10, char('A' + rand() % 26));
}

TEST("hash_map fuzzy test") {
    srand(time(nullptr));

    auto um = std::unordered_map<std::string, std::string> {};
    auto hm = hash_map<std::string, std::string> {};

    // bunch of random actions
    for (auto i = 0u; i < 100'000u; i++) {
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
                auto e = um.begin(); for (auto i = 0u; i < n; i++) e++;
                auto k = e->first;
                um.erase(k);
                hm.erase(k);
            }
            break;
        case 2:
            // update random element
            if (um.size()) {
                auto n = rand() % um.size();
                auto e = um.begin(); for (auto i = 0u; i < n; i++) e++;
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
        ASSERT(hm.find(k)->second == v);
    }
}


TEST("hashmap vs unordered_map string lots of inserts") {
    const auto FUZZY_SIZE = 10'000;
    const auto keys = random_keys(FUZZY_SIZE);

    // perf test map
    auto benchmark = [&](auto container, auto name) {
        timed(name) {
            for (auto& k: keys) {
                container.insert_or_assign(k, 100);
            }
        };
    };

    auto hm = hash_map<std::string_view, int, std::hash<std::string_view>> {};
    auto um = std::unordered_map<std::string_view, int> {};
    benchmark(hm, "hash_map string inserts");
    benchmark(um, "unordered_map string inserts");
}

TEST("hashmap vs unordered_map lots of lookups with sum") {
    const auto FUZZY_SIZE = 50'000;
    const auto keys = random_keys(FUZZY_SIZE);

    auto benchmark = [&](auto container, auto name) {
        srand(12345);
        timed(name) {
            auto total = 0;
            for (auto i = 0; i < FUZZY_SIZE; i++) {
                total += container.at(keys[rand() % keys.size()]);
            }
            log("total: ", total);
        };
    };

    auto hm = hash_map<std::string_view, int, std::hash<std::string_view>> {};
    auto um = std::unordered_map<std::string_view, int> {};
    for (auto& k: keys) {
        hm.insert_or_assign(k, k.size());
        um.insert_or_assign(k, k.size());
    }
    benchmark(hm, "hash_map lookups");
    benchmark(um, "unordered_map lookups");
}

TEST("hashmap vs unordered_map iteration") {
    const auto FUZZY_SIZE = 10'000;
    const auto keys = random_keys(FUZZY_SIZE);

    auto hm = hash_map<std::string_view, int, std::hash<std::string_view>> {};
    auto um = std::unordered_map<std::string_view, int> {};
    for (auto& k: keys) {
        hm.insert_or_assign(k, k.size());
        um.insert_or_assign(k, k.size());
    }
    auto benchmark = [&](auto container, auto name) {
        srand(12345);
        timed(name) {
            auto total = 0;
            for (auto& [ k, v ]: container) {
                total += v;
            }
            log("total: ", total);
        };
    };

    benchmark(hm, "hash_map iteration");
    benchmark(um, "unordered_map iteration");

}


TEST("hashmap many random ops vs unordered_map") {
    const auto FUZZY_SIZE = 200'000;
    auto hm = hash_map<int, int> {};
    auto um = std::unordered_map<int, int> {};

    enum Op {
        Construct = 0,
        ConstructInitList = 1,
        ConstructCopy = 2,
        ConstructMove = 3,

        Clear = 4,
        Contains = 5,
        Find = 6,
        At = 7,
        Insert = 8,
        Index = 9,
        Remove = 10,

        Size = 11,
        Empty = 12,

        AssignCopy = 13,
        AssignMove = 14,

        MAX
    };

    auto NUM = [=] { return rand() % FUZZY_SIZE; };
    srand(1025);
    auto maxsize = 0ul;

    for (auto i = 0; i < FUZZY_SIZE; i++) {
        const auto op = rand() % Op::MAX;
        switch (op) {
            case Op::Construct: {
                // hm = hash_map<int, int> {};
                // um = std::unordered_map<int, int> {};
                // break;
            }
            case Op::ConstructCopy: {
                auto b = hm;
                hm = hash_map<int, int>(b);
                break;
            }
            case Op::AssignCopy: { break; }
            case Op::AssignMove: { break; }
            case Op::ConstructMove: {
                auto b = std::move(hm);
                hm = hash_map<int, int>(std::move(b));
                break;
            }
            case Op::ConstructInitList: { break; }
            // case Op::Clear: {
            //     if (rand() % 100 == 0) {
            //         // hm.clear();
            //         // um.clear();
            //     }
            //     break;
            // }
            case Op::Size: { ASSERT(hm.size() == um.size()); break; }
            case Op::Empty: { ASSERT(hm.empty() == um.empty()); break; }
            case Op::Contains: { auto n = NUM(); ASSERT(hm.contains(n) == um.contains(n)); break; }
            case Op::Find: {
                auto n = NUM();
                if (um.find(n) == um.end()) {
                    ASSERT(hm.find(n) == hm.end());
                } else {
                    ASSERT(hm.find(n) != hm.end());
                }
                break;
            }
            case Op::At: {
                auto n = NUM();
                if (um.contains(n)) {
                    try {
                        ASSERT(hm.at(n) == um.at(n));
                    } catch (...) {
                        throw std::runtime_error {"sdf"};
                    }
                } else {
                    ASSERT_THROWS(hm.at(n));
                }
                break;
            }
            case Op::Insert: {
                auto k = NUM(), v = NUM();
                hm.insert_or_assign(k, v);
                um.insert_or_assign(k, v);
                break;
            }
            case Op::Index: {
                auto k = NUM();
                if (um.contains(k)) {
                    ASSERT(um.at(k) == hm.at(k));
                } else {
                    ASSERT(!hm.contains(k));
                }
                break;
            }
            case Op::Remove: {
                auto k = NUM();
                ASSERT(um.erase(k) == hm.erase(k));
                break;
            }
            default: break;
        }
        maxsize = std::max(maxsize, hm.size());
    }

    ASSERT(um.size() == hm.size());
    for (auto [ k, v ]: um) {
        ASSERT(v == hm.at(k));
    }

    log("max size: ", maxsize);
}
