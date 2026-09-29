#include "jlib/hash_table.h"
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
    timed_timer(auto f, auto name) {
        auto before = stdc::steady_clock::now();
        f();
        auto after = stdc::steady_clock::now();
        log(Colors::FG_YELLOW2, "timer:", name, Colors::FG_DEFAULT, stdc::duration_cast<stdc::microseconds>(after - before).count());
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


// TEST("hashmap vs unordered_map string lots of inserts") {
//     const auto FUZZY_SIZE = 10'000;
//     const auto keys = random_keys(FUZZY_SIZE);

//     // perf test map
//     auto benchmark = [&](auto container, auto name) {
//         timed(name) {
//             for (auto& k: keys) {
//                 container.emplace(k, 100);
//             }
//         };
//     };

//     auto hm = hash_map<std::string_view, int, std::hash<std::string_view>> {};
//     auto um = std::unordered_map<std::string_view, int> {};
//     benchmark(hm, "hash_map string inserts");
//     benchmark(um, "unordered_map string inserts");
// }

// TEST("hashmap vs unordered_map lots of lookups with sum") {
//     const auto FUZZY_SIZE = 50'000;
//     const auto keys = random_keys(FUZZY_SIZE);

//     auto benchmark = [&](auto container, auto name) {
//         srand(12345);
//         timed(name) {
//             auto total = 0;
//             for (auto i = 0; i < FUZZY_SIZE; i++) {
//                 total += container.at(keys[rand() % keys.size()]);
//             }
//             log("total: ", total);
//         };
//     };

//     auto hm = hash_map<std::string_view, int, std::hash<std::string_view>> {};
//     auto um = std::unordered_map<std::string_view, int> {};
//     for (auto& k: keys) {
//         hm.emplace(k, k.size());
//         um.emplace(k, k.size());
//     }
//     benchmark(hm, "hash_map lookups");
//     benchmark(um, "unordered_map lookups");
// }

// TEST("hashmap vs unordered_map iteration") {
//     const auto FUZZY_SIZE = 10'000;
//     const auto keys = random_keys(FUZZY_SIZE);

//     auto hm = hash_map<std::string_view, int, std::hash<std::string_view>> {};
//     auto um = std::unordered_map<std::string_view, int> {};
//     for (auto& k: keys) {
//         hm.emplace(k, k.size());
//         um.emplace(k, k.size());
//     }
//     auto benchmark = [&](auto container, auto name) {
//         srand(12345);
//         timed(name) {
//             auto total = 0;
//             for (auto& [ k, v ]: um) {
//                 total += v;
//             }
//             log("total: ", total);
//         };
//     };

//     benchmark(hm, "hash_map iteration");
//     benchmark(um, "unordered_map iteration");

// }

TEST("hashmap many random ops vs unordered_map") {
    log("\nmany random ops vs unordered_map");
    const auto FUZZY_SIZE = 1000000;
    auto hm = hash_table<int, int> {};
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

    auto NUM = [=] { return rand() % 100; };
    srand(1025);
    auto maxsize = 0ul;

    for (auto i = 0; i < 100000; i++) {
        const auto op = rand() % Op::MAX;
        switch (op) {
            case Op::Construct: {
                hm = hash_table<int, int> {};
                um = std::unordered_map<int, int> {};
                break;
            }
            case Op::ConstructCopy: {
                auto b = hm;
                hm = hash_table<int, int>(b);
                break;
            }
            case Op::ConstructInitList: { break; }
            case Op::ConstructMove: {
                auto b = std::move(hm);
                hm = hash_table<int, int>(std::move(b));
                break;
            }
            case Op::Clear: {
                if (rand() % 100 < 5) {
                    hm.clear();
                    um.clear();
                }
                break;
            }
            case Op::Size: {
                ASSERT(hm.size() == um.size());
                break;
            }
            case Op::Empty: {
                ASSERT(hm.empty() == um.empty());
                break;
            }
            case Op::Contains: {
                auto n = NUM();
                ASSERT(hm.contains(n) == um.contains(n));
                break;
            }
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
                log("op at: ", n);
                if (um.contains(n)) {
                    try {
                        log("um: ", um.at(n));
                        log("hm: ", hm.at(n));
                        ASSERT(hm.at(n) == um.at(n));
                    } catch (...) {
                        // DUMP();
                        log("not equal: ", n);
                        throw std::runtime_error {"sdf"};
                    }
                } else {
                    ASSERT_THROWS(hm.at(n));
                }
                break;
            }
            case Op::Insert: {
                auto k = NUM(), v = NUM();
                log("insert:", k, v);
                if (k == 44) {
                    log("insert 44");
                }
                hm.insert_or_assign(k, v);
                um.insert_or_assign(k, v);
                // DUMP();
                ASSERT(hm.size() == um.size());
                break;
            }
            case Op::Index: {
                auto k = NUM();
                // log("contains: ", k);
                // if (k == 35) { DUMP(); }
                if (um.contains(k)) {
                    ASSERT(um.at(k) == hm.at(k));
                } else {
                    ASSERT(!hm.contains(k));
                }
                break;
            }
            case Op::Remove: {
                auto k = NUM();
                log("remove:", k);
                ASSERT(um.erase(k) == hm.erase(k));
                ASSERT(hm.size() == um.size());
                break;
            }
            case Op::AssignCopy: { break; }
            case Op::AssignMove: { break; }
            default: break;
        }
    }

    ASSERT(um.size() == hm.size());
    for (auto [ k, v ]: um) {
        ASSERT(v == hm.at(k));
    }

    log("max size: ", maxsize);
}
