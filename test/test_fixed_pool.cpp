// test_fixed_pool.cpp
#include <jlib/test_framework.h>


#include <jlib/fixed_pool.h>
#include <string>

TEST("fixed pool simple") {
    auto p = fixed_pool<int>(100);
    p.add(1);
    p.add(2);
    p.add(3);

    auto vec = std::vector<int> { std::begin(p), std::end(p) };
    auto vec2 = std::vector { 1, 2, 3 };
    ASSERT(vec == vec2);
}
TEST("fixed pool add/remove") {
    auto p = fixed_pool<int>(100);

    p.add(1);
    p.add(2);
    auto& r = p.add(3);
    p.add(4);
    p.add(5);

    p.remove(r);

    ASSERT(std::vector(p.begin(), p.end()) == std::vector { 1, 2, 4, 5 });
}

TEST("fixed pool several random add/remove compare to vector") {
    srand(time(nullptr));
    const auto N = 10'000;
    const auto M = 10000;
    
    auto p = fixed_pool<int>{M};
    auto v = std::vector<int>{};
    auto nextval = 1010;

    auto inserts = 0;
    auto remove_full = 0;
    auto remove_random = 0;
    
    for (auto i = 0; i < N; i++) {
        if (p.count() == p.capacity()) {
            // remove random full
            const auto index = rand() % p.capacity();
            const auto val = p.storage.at(index);
            p.remove(p.storage.at(index));
            std::erase_if(v, [&](auto x) { return x == val;});
            remove_full++;
        } else if (rand() % 10 == 1 && p.count() > 0) {
            const auto index = rand() % p.capacity();
            // remove random if busy
            if (p.state.at(index) == decltype(p)::State::Busy) {
                const auto val = p.storage.at(index);
                p.remove(p.storage.at(index));
                std::erase_if(v, [&](auto x) { return x == val;});
                remove_random++;
            }
        } else {
            // add random
            // insert into the tracking vector at the correct index
            const auto val = nextval++;
            auto& t = p.add(val);
            auto index = &t - p.storage.data();
            auto free_before = 0;
            for (auto i = 0; i < index; i++) { free_before += (p.state.at(i) == decltype(p)::State::Free); }
            v.insert(v.begin() + index - free_before, val);
            inserts++;
        }
    }

    v.resize(p.count());
    const auto pv = std::vector<int>(p.begin(), p.end());

    log("inserts:", inserts, "removes:", remove_random, "removes[f]: ", remove_full);
    ASSERT(v == pv);
}

// TEST("fixed pool") {
//     // auto p = fixed_pool<int>(100);

//     // p.add(1);
//     // p.add(2);
//     // auto& r = p.add(3);

//     // p.remove(&r);

//     // for (auto i : p) {
//     //     log(i);
//     // }


//     // auto vec = std::vector<int> { p.begin(), p.end() };
//     // auto vec2 = std::vector { 1, 2 };

//     // log(vec, vec2);

//     // ASSERT(vec == vec2);
// }


// TEST("lots of random adds compared to vector") {
//     // const auto N = 1024;
//     // const auto M = 1100;
//     // auto p = fixed_pool<int>(N);
//     // auto v = std::vector<int>{}; v.reserve(N);

//     // srand(1024);
//     // for (auto i = 0; i < M; i++) {
//     //     if (p.has_space()) {
//     //         // add a random element
//     //         auto e = rand();
//     //         p.add(e);
//     //         v.emplace_back(e);
//     //     }
//     // }

//     // ASSERT(std::vector<int>(p.begin(), p.end()) == v);
// }

// TEST("lots of random adds and removes compared to vector") {
//     // const auto N = 1024;
//     // const auto M = 10000;
//     // auto p = fixed_pool<int>(N);
//     // auto v = std::vector<int>{}; v.reserve(N);

//     // auto* startptr = &p.add(0);
//     // v.emplace_back(0);

//     // auto MAXSIZE = 0u;

//     // srand(25);
//     // for (auto i = 0; i < M; i++) {
//     //     const auto r = rand() % 2;
//     //     if (r == 0) {
//     //         if (p.has_space()) {
//     //             // add a random element
//     //             // use pointer to determine where the element was added
//     //             auto e = rand();
//     //             auto* ptr = &p.add(e);
//     //             auto index = std::distance(startptr, ptr);
//     //             v.insert(v.begin() + index - p.count_free_before(index), e);
//     //         }
//     //     } else {
//     //         auto index = rand() % (p.count() + 1);
//     //         auto ptr = startptr + index;
//     //         if (p.is_busy(index)) {
//     //             p.remove(ptr);
//     //             // index -= p.count_free_before(index);
//     //             v.erase(v.begin() + index - p.count_free_before(index));
//     //         }
//     //     }

//     //     ASSERT(p.count() == v.size());
//     //     MAXSIZE = std::max(MAXSIZE, p.gaps());
//     //     // log(v.size(), p.gaps()); //p.count_free_before(1024));
//     // }

//     // ASSERT(std::vector<int>(p.begin(), p.end()) == v);
//     // log("largest gap count: ", MAXSIZE);
// }
