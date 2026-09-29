// test_fixed_pool.cpp
#include <jlib/test_framework.h>
#include <jlib/object_pool.h>

#include <numeric>
using namespace std::literals;

TEST("object pool constructors/CTAD") {
    auto v1 = dynamic_pool { 1, 2, 3 };
    const auto c = v1.collect();
    ASSERT(c == std::vector { 1, 2, 3 });

    auto v4 = std::vector(v1.begin(), v1.end());
    ASSERT(v4 == std::vector { 1, 2, 3 });

    auto v2 = dynamic_pool { 1 };
    ASSERT(v2.collect() == std::vector { 1 });

    auto v3 = dynamic_pool { "hello"s, "world"s };
    ASSERT(v3.collect() == std::vector { "hello"s, "world"s });
}

TEST("object pool add") {
    auto p = dynamic_pool<int>{};
    p.add(1);
    p.add(2);
    p.add(3);

    ASSERT(p.collect() == std::vector { 1, 2, 3 });
}
TEST("object pool add/remove") {
    auto p = dynamic_pool<int>{};

    p.add(1);
    p.add(2);
    p.add(3);
    p.add(4);
    p.add(5);

    p.remove(2);

    ASSERT(p.collect() == std::vector { 1, 2, 4, 5 });

    p.remove(3);

    ASSERT(p.collect() == std::vector { 1, 2, 5 });
}


TEST("object_pool begin/end") {
    auto vec = object_pool { 1, 2, 3, 4, 5, 6 };

    ASSERT(vec.index_of(vec.begin()) == 0);
    ASSERT(vec.index_of(vec.end()) == 6);

    vec.remove(5);
    ASSERT(vec.index_of(vec.begin()) == 0);
    ASSERT(vec.index_of(vec.end()) == 5);

    vec.remove(0);
    ASSERT(vec.index_of(vec.begin()) == 1);
    ASSERT(vec.index_of(vec.end()) == 5);

    vec.remove(4);
    ASSERT(vec.index_of(vec.begin()) == 1);
    ASSERT(vec.index_of(vec.end()) == 4);

    vec.remove(2);
    ASSERT(vec.index_of(vec.begin()) == 1);
    ASSERT(vec.index_of(vec.end()) == 4);

    vec.remove(1);
    auto b = vec.index_of(vec.begin());
    ASSERT(b == 3);
    ASSERT(vec.index_of(vec.end()) == 4);
}



TEST("remove from middle, push two, size didn't change") {
    auto vec = dynamic_pool { 1, 2, 3, 4 };
    vec.add(5);
    vec.remove(2);
    vec.remove(3);
    vec.add(6);
    vec.add(7);
    ASSERT(vec.collect() == std::vector { 1, 2, 7, 6, 5 });
}


TEST("pool with some removes") {
    auto vec = dynamic_pool { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    vec.remove(3);
    vec.remove(6);
    vec.add(32);
    vec.remove(7);
    vec.add(64);

    ASSERT(vec.collect() == std::vector { 1, 2, 3, 5, 6, 32, 64, 9, 10 }); //
}


TEST("object pool write iterator") {
    auto p = dynamic_pool { 1, 2, 3, 4, 5 };
    for (auto& i: p) { i *= i; }
    ASSERT(p.collect() == std::vector { 1, 4, 9, 16, 25 });
}

TEST("object pool init list") {
    const auto fp = dynamic_pool { 1, 2, 3, 4, 5 };
    ASSERT(fp.collect() == std::vector { 1,2,3,4,5 });
}

TEST("object pool sum") {
    auto fp = dynamic_pool { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    fp.remove_if([](auto x) { return x == 1 || x == 5; });
    ASSERT(std::accumulate(fp.begin(), fp.end(), 0) == 49);
}

TEST ("object pool remove while iterate") {
    auto fp = fixed_pool { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    for (auto i = fp.begin(); i != fp.end(); i++) {
        if ((*i & 1) != 0) {
            fp.remove(i);
        }
    }
    ASSERT(fp.collect() == std::vector { 2, 4, 6, 8, 10 });
}

TEST("dynamic pool remove while iterate fuzzy") {
    auto fp = dynamic_pool { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    for (auto i = 0; i < 40; i++) {
        if (rand() & 1) {
            fp.remove(rand() % fp.width());
        } else {
            fp.add(rand() % 100);
        }
    }
    fp.remove(11); fp.remove(10);

    for (auto i = fp.begin(); i < fp.end(); i++) {
        if ((*i & 1) != 0) {
            fp.remove(i);
        }
    }

    for (auto i = fp.begin(); i < fp.end(); i++) {
        ASSERT((*i & 1) == 0);
    }
}

TEST ("object pool remove while iterate 2") {
    auto fp = fixed_pool<char> { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j' };
    for (auto i = fp.begin(); i < fp.end(); i++) {
        if ((*i & 1) == 0) {
            fp.remove(i);
        }
        if (fp.index_of(i) > 20) break;
    }
    ASSERT(fp.collect() == std::vector<char> { 'a', 'c', 'e', 'g', 'i' });
}

TEST("object pool remove_if") {
    auto fp = fixed_pool { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    fp.remove_if([](auto x) { return x & 1; });
    ASSERT(fp.collect() == std::vector { 2, 4, 6, 8, 10 });
}
TEST("object pool remove_if") {
    // remove odd
    auto vec = dynamic_pool { 1, 2, 3, 4, 5, 6, 7 };
    vec.remove_if([](auto i) { return i & 1; });
    ASSERT(vec.collect() == std::vector { 2, 4, 6 });

    // remove even
    vec = dynamic_pool { 1, 2, 3, 4, 5, 6, 7 };
    vec.remove_if([](auto i) { return (i+1) & 1; });
    ASSERT(vec.collect() == std::vector { 1, 3, 5, 7 });

    // remove from start
    vec = dynamic_pool { 1, 2, 3, 4, 5, 6, 7 };
    vec.remove_if([](auto i) { return i < 3; });
    ASSERT(vec.collect() == std::vector { 3, 4, 5, 6, 7 });

    // remove from end
    vec = dynamic_pool { 1, 2, 3, 4, 5, 6, 7 };
    vec.remove_if([](auto i) { return i > 5; });
    ASSERT(vec.collect() == std::vector { 1, 2, 3, 4, 5 });

    // remove all
    vec = dynamic_pool { 1, 2, 3, 4, 5, 6, 7 };
    vec.remove_if([](int) { return true; });
    ASSERT(vec.collect() == std::vector<int>{});

    // remove none
    vec = dynamic_pool { 1, 2, 3, 4, 5 };
    vec.remove_if([] (int) { return false; });
    ASSERT(vec.collect() == std::vector { 1, 2, 3, 4, 5 });
}

TEST("object pool range iteration") {
    auto vec = dynamic_pool { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    vec.remove(1);
    vec.remove(3);
    vec.remove(5);

    auto total = 0;
    for (auto i: vec) {
        total += i;
    }
    ASSERT(total == 43);
}


TEST("object pool remove from end, free slots not used") {
    auto vec = object_pool { 1, 2, 3, 4, 5, 6 };
    vec.remove(5);
    vec.remove(4);

    constexpr auto x = sizeof(std::vector<int>);
    auto& FREE = *(std::vector<size_t>*)(((char*)&vec) + sizeof(std::vector<int>) + sizeof(std::vector<bool>));

    ASSERT(FREE.size() == 0);
    ASSERT(FREE.capacity() == 6);
}

TEST("object pool several random add/remove compare to vector") {
    return;
    srand(time(nullptr));
    const auto N = 10'000uz;
    const auto M = 5'000uz;
    
    auto p = dynamic_pool<int>(M);
    auto v = std::vector<int>{};
    auto nextval = 1010;

    auto inserts = 0;
    auto remove_full = 0;
    auto remove_random = 0;
    
    for (auto i = 0u; i < N; i++) {
        if (p.count() == p.capacity()) {
            // remove random full
            const auto index = rand() % p.capacity();
            const auto val = p.at(index);
            p.remove(p.at(index));
            std::erase_if(v, [&](auto x) { return x == val;});
            remove_full++;
        } else if (rand() % 10 == 1 && p.count() > 0) {
            // remove random if busy
            const auto index = rand() % p.capacity();
            if (p.is_busy(index)) {
                const auto val = p.at(index);
                p.remove(index);
                std::erase_if(v, [&](auto x) { return x == val;});
                remove_random++;
            }
        } else {
            // add random
            // insert into the tracking vector at the correct index
            const auto val = nextval++;
            const auto index = p.add(val);
            auto free_before = 0;
            for (auto i = 0; i < index; i++) { free_before += (p.is_busy(i) ? 0 : 1); }
            v.insert(v.begin() + index - free_before, val);
            inserts++;
        }
    }

    v.resize(p.count());

    log("\ninserts:", inserts, "removes:", remove_random, "removes[f]: ", remove_full);

    ASSERT(inserts > 0);
    ASSERT(remove_full > 0);
    ASSERT(remove_random > 0);
    ASSERT(v == p.collect());
}
