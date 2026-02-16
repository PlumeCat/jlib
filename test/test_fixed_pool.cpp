// test_fixed_pool.cpp
#include <jlib/test_framework.h>
#include <jlib/fixed_pool.h>
#include <string>

TEST("fixed pool") {
    auto p = fixed_pool<int>(100);

    p.add(1);
    p.add(2);
    auto& r = p.add(3);

    p.remove(&r);

    for (auto i : p) {
        log(i);
    }


    auto vec = std::vector<int> { p.begin(), p.end() };
    auto vec2 = std::vector { 1, 2 };

    log(vec, vec2);

    ASSERT(vec == vec2);
}


TEST("lots of random adds compared to vector") {
    const auto N = 1024;
    const auto M = 1100;
    auto p = fixed_pool<int>(N);
    auto v = std::vector<int>{}; v.reserve(N);

    srand(1024);
    for (auto i = 0; i < M; i++) {
        if (p.has_space()) {
            // add a random element
            auto e = rand();
            p.add(e);
            v.emplace_back(e);
        }
    }

    ASSERT(std::vector<int>(p.begin(), p.end()) == v);
}

TEST("lots of random adds and removes compared to vector") {
    const auto N = 1024;
    const auto M = 10000;
    auto p = fixed_pool<int>(N);
    auto v = std::vector<int>{}; v.reserve(N);

    auto* startptr = &p.add(0);
    v.emplace_back(0);

    auto MAXSIZE = 0u;

    srand(25);
    for (auto i = 0; i < M; i++) {
        const auto r = rand() % 2;
        if (r == 0) {
            if (p.has_space()) {
                // add a random element
                // use pointer to determine where the element was added
                auto e = rand();
                auto* ptr = &p.add(e);
                auto index = std::distance(startptr, ptr);
                v.insert(v.begin() + index - p.count_free_before(index), e);
            }
        } else {
            auto index = rand() % (p.count() + 1);
            auto ptr = startptr + index;
            if (p.is_busy(index)) {
                p.remove(ptr);
                // index -= p.count_free_before(index);
                v.erase(v.begin() + index - p.count_free_before(index));
            }
        }

        ASSERT(p.count() == v.size());
        MAXSIZE = std::max(MAXSIZE, p.gaps());
        // log(v.size(), p.gaps()); //p.count_free_before(1024));
    }

    ASSERT(std::vector<int>(p.begin(), p.end()) == v);
    log("largest gap count: ", MAXSIZE);
}
