#include "jlib/hash_map.h"
#include "jlib/test_framework.h"
#include "jlib/timer.h"

#include <unordered_map>

// // random key generator
// std::string random_ascii_string() {
//     const auto len = 10 + rand() % 10;
//     auto key = std::string(len, 'a');
//     for (auto i = 0; i < len; i++) {
//         key[i] = rand() % 127 + 32;
//     }
//     return key;
// }
// std::vector<std::string> random_keys(int n) {
//     auto keys = std::vector<std::string> {};
//     for (auto i = 0; i < n; i++) {
//         keys.emplace_back(random_ascii_string());
//     }
//     return keys;
// }

// struct timed_timer {
//     timed_timer(auto f, auto name) {
//         auto before = std::chrono::steady_clock::now();
//         f();
//         auto after = std::chrono::steady_clock::now();
//         log(Colors::FG_YELLOW2, "timer:", name, Colors::FG_DEFAULT, std::chrono::duration_cast<std::chrono::microseconds>(after - before).count());
//     }
// };
// struct timed_dummy { std::string_view name; };
// template <typename F>
// auto operator<<(timed_dummy d, F f) { return timed_timer(f, d.name); }
// #ifndef paste
// #define paste(a, b) a##b
// #endif
// #define _timed(name, c) auto paste(_timed_timer, c) = timed_dummy { name } << [&]
// #define timed(name) _timed(name, __COUNTER__)


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


// TEST("hashmap many random ops vs unordered_map")
// {
//     const auto FUZZY_SIZE = 1000000;
//     auto hm = hash_map<int, int> {};
//     auto um = std::unordered_map<int, int> {};

//     enum Op {
//         Construct = 0,
//         ConstructInitList,
//         ConstructCopy,
//         ConstructMove,

//         Clear,
//         Contains,
//         Find,
//         At,
//         Insert,
//         Index,
//         Remove,

//         Size,
//         Empty,

//         AssignCopy,
//         AssignMove,

//         MAX
//     };

//     auto NUM = [=] { return rand() % 100; };
//     auto DUMP = [&] {
//         auto k1 = std::vector<int> {};

//         for (auto [ k, v ]: hm) {
//             k1.push_back(k);
//         }

//         auto k2 = std::vector<int> {};
//         for (auto [ k, v ]: um) {
//             k2.push_back(k);
//         }

//         std::sort(k1.begin(), k1.end());
//         std::sort(k2.begin(), k2.end());

//         for (auto k: k1) {
//             log("hm: ", k, hm[k]);
//         }
//         for (auto k: k2) {
//             log("um: ", k, hm[k]);
//         }

//     };

//     srand(1025);
//     auto maxsize = 0ul;
//     for (auto i = 0; i < FUZZY_SIZE; i++) {
//         const auto op = rand() % Op::MAX;
//         switch (op) {
//             case Op::Construct: {
//                 hm = hash_map<int, int> {};
//                 um = std::unordered_map<int, int> {};
//                 break;
//             }
//             case Op::ConstructInitList: {
//                 auto il = std::initializer_list<std::pair<int, int>> {
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                     { NUM(), NUM() },
//                 };
//                 hm = hash_map<int, int> { il };
//                 um = std::unordered_map<int, int> { il.begin(), il.end() };
//                 break;
//             }
//             case Op::ConstructCopy: {
//                 auto b = hm;
//                 hm = hash_map<int, int>(b);
//                 break;
//             }
//             case Op::ConstructMove: {
//                 auto b = hm;
//                 hm = hash_map<int, int>(std::move(b));
//                 break;
//             }
//             case Op::Clear: {
//                 hm.clear();
//                 um.clear();
//                 break;
//             }
//             case Op::Size: {
//                 ASSERT(hm.size() == um.size());
//                 maxsize = std::max(maxsize, hm.size());
//                 break;
//             }
//             case Op::Empty: {
//                 ASSERT(hm.empty() == um.empty());
//                 break;
//             }
//             case Op::Contains: {
//                 auto n = NUM();
//                 ASSERT(hm.contains(n) == um.contains(n));
//                 break;
//             }
//             case Op::Find: {
//                 auto n = NUM();
//                 if (um.find(n) != um.end()) {
//                     ASSERT(hm.find(n) != hm.end());
//                 } else {
//                     ASSERT(hm.find(n) == hm.end());
//                 }
//                 break;
//             }
//             case Op::At: {
//                 auto n = NUM();
//                 if (um.contains(n)) {
//                     ASSERT(hm.at(n) == um.at(n));
//                 } else {
//                     ASSERT_THROWS(hm.at(n));
//                 }
//                 break;
//             }
//             case Op::Insert: {
//                 auto k = NUM(), v = NUM();
//                 um.emplace(k, v);
//                 hm.emplace(k, v);
//                 break;
//             }
//             case Op::Index: {
//                 auto k = NUM();
//                 ASSERT(um[k] == hm[k]);
//                 break;
//             }
//             case Op::Remove: {
//                 auto k = NUM();
//                 ASSERT(um.erase(k) == hm.erase(k));
//                 break;
//             }
//             case Op::AssignCopy: { break; }
//             case Op::AssignMove: { break; }
//             default: break;
//         }
//     }

//     ASSERT(um.size() == hm.size());
//     for (auto [ k, v ]: um) {
//         ASSERT(v == hm.at(k));
//     }

//     log("max size: ", maxsize);
// }
