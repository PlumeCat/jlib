#include <string>
#include <jlib/hash_map.h>
#include <jlib/test_framework.h>

using namespace std;


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

    for (const auto& [ k, v ]: h) {
        ASSERT(h[k] == v);
        // log(k, v);
    }

}


TEST("integer hashmap") {
    auto h = hash_map<uint32_t, string> {};

    h[0] = "ZERO.";
    h[1] = "one";
    h[3] = "three";
    h[534] = "five hundred and thirty four";

    for (const auto& [ k, v ]: h) {
        ASSERT(h[k] == v);
        // log(k, v);
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
        { Foo::Bar, "Foo.Bar" },
        { Foo::Baz, "Foo.Baz" },
        { Foo::Quz, "Foo.Quz" },
        { Foo::Quux, "Foo.Quux" },
    };

    for (const auto& [ k, v ]: FooName) {
        // log((uint32_t)k, v);
        ASSERT(FooName[k] == v);
    }
}

