#include <jlib/hash_map.h>
// #include <jlib/hash_map2.h>
#include <jlib/test_framework.h>
#include <string>

using namespace std;

TEST("hashmap 2") {}

TEST("hashmap") {
    auto h = hash_map<string, string>(32);

    h["hello"] = "world";
    h["foo"] = "bar";
    h["baz"] = "quz";
    h["this"] = "is";

    ASSERT(h.contains("foo"));
    ASSERT(h.contains("hello"));

    h.remove("foo");

    ASSERT(!h.contains("foo"))
    ASSERT(h.contains("baz"))

    ASSERT(!h.empty());
    ASSERT(h.find("hello") != h.end());

    for (const auto& [k, v] : h) {
        ASSERT(h[k] == v);
    }
}

TEST("integer hashmap") {
    auto h = hash_map<uint32_t, string> {};

    h[0] = "ZERO.";
    h[1] = "one";
    h[3] = "three";
    h[534] = "five hundred and thirty four";

    for (const auto& [k, v] : h) {
        ASSERT(h[k] == v);
    }

    ASSERT_THROWS(h.remove(5));

    h[0] = "ZEROZEROZERO";
    log(h[0]);

    ASSERT(h.contains(1));
    ASSERT(!h.contains(2));
}

TEST("enum hashmap") {
    enum class Foo : uint64_t {
        Bar = 14,
        Baz = 1937,
        Quz = 32,
        Quux,
    };

    auto FooName = hash_map<Foo, string> {
        {Foo::Bar,   "Foo.Bar" },
        { Foo::Baz,  "Foo.Baz" },
        { Foo::Quz,  "Foo.Quz" },
        { Foo::Quux, "Foo.Quux"},
    };

    for (const auto& [k, v] : FooName) {
        // log((uint32_t)k, v);
        ASSERT(FooName[k] == v);
    }
}

TEST("iterate hashmap") {
    auto h = hash_map<string, double> {
        {"x",  1},
        { "y", 2},
        { "z", 3},
        { "w", 4}
    };

    for (const auto& [k, v] : h) {
        log(k, v);
    }
}

#include <unordered_map>

TEST("fuzzy test vs unordered_map") {

    auto hm = hash_map<std::string, int> {};
    auto um = std::unordered_map<std::string, int> {};

    log("before map");

    srand(1024); //
    for (auto i = 0; i < 100'000; i++) {
        auto key = 'k' + std::to_string(rand()) + '-' + std::to_string(rand());
        auto val = rand();

        hm[key] = val;
        um[key] = val;
    }

    log("map: ", hm.size(), um.size());

    for (auto& [k, v] : um) {
        ASSERT(hm[k] == v);
    }
}
