// test_fixed_pool.cpp
#include <jlib/test_framework.h>


#include <jlib/fixed_pool.h>
#include <string>

TEST("fixed pool simple") {
    auto p = fixed_pool<int>(100);
    p.add(1);
    p.add(2);
    p.add(3);

    ASSERT(p.collect() == std::vector { 1, 2, 3 });
}
TEST("fixed pool add/remove") {
    auto p = fixed_pool<int>(100);

    p.add(1);
    p.add(2);
    auto& r = p.add(3);
    p.add(4);
    p.add(5);

    p.remove(r);

    ASSERT(p.collect() == std::vector { 1, 2, 4, 5 });

    p.remove(p.get_storage().at(3));

    ASSERT(p.collect() == std::vector { 1, 2, 5 });
}

TEST("fixed pool init list") {
    const auto fp = fixed_pool { 1, 2, 3, 4, 5 };
    ASSERT(fp.collect() == std::vector { 1,2,3,4,5 });
}

TEST("fixed pool erase while iterate") {
    auto fp = fixed_pool { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    for (auto i = fp.begin(); i != fp.end(); i++) {
        if (!(*i & 1)) {
            fp.remove(i);
            fp.remove(i);
        }
    }
    log(fp.collect());
    ASSERT(fp.collect() == std::vector { 1, 3, 5, 7, 9 });
}

TEST("fixed pool remove_if") {
    auto fp = fixed_pool { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    fp.remove_if([](auto x) { return x & 1; });
    ASSERT(fp.collect() == std::vector { 2, 4, 6, 8, 10 });
}

TEST("fixed pool several random add/remove compare to vector") {
    srand(time(nullptr));
    const auto N = 10'000u;
    const auto M = 5'000u;
    
    auto p = fixed_pool<int>(M);
    auto v = std::vector<int>{};
    auto nextval = 1010;

    auto inserts = 0;
    auto remove_full = 0;
    auto remove_random = 0;

    log("FP fuzz");
    
    for (auto i = 0; i < N; i++) {
        if (p.count() == p.capacity()) {
            // remove random full
            const auto index = rand() % p.capacity();
            const auto val = p.get_storage().at(index);
            p.remove(p.get_storage().at(index));
            std::erase_if(v, [&](auto x) { return x == val;});
            remove_full++;
        } else if (rand() % 10 == 1 && p.count() > 0) {
            // remove random if busy
            const auto index = rand() % p.capacity();
            if (p.is_busy(index)) {
                const auto val = p.get_storage().at(index);
                p.remove(p.get_storage().at(index));
                std::erase_if(v, [&](auto x) { return x == val;});
                remove_random++;
            }
        } else {
            // add random
            // insert into the tracking vector at the correct index
            const auto val = nextval++;
            const auto index = &p.add(val) - p.get_storage().data();
            auto free_before = 0;
            for (auto i = 0; i < index; i++) { free_before += (p.is_busy(i) ? 0 : 1); }
            v.insert(v.begin() + index - free_before, val);
            inserts++;
        }
    }

    v.resize(p.count());

    log("inserts:", inserts, "removes:", remove_random, "removes[f]: ", remove_full);
    ASSERT(v == p.collect());
    ASSERT(inserts > 0);
    ASSERT(remove_full > 0);
    ASSERT(remove_random > 0);
}
