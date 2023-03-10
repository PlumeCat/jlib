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


#define jenum_name Foo
#define jenum() \
    jenum_entry(A)\
    jenum_entry(B)\
    jenum_entry(C)\
    jenum_entry(D)\
    jenum_entry(E)\
    jenum_entry(F)\
    jenum_entry(G)\
    jenum_entry(H)\
    jenum_entry(I)\
    jenum_entry(J)\
    jenum_entry(K)\
    jenum_entry(L)\
    jenum_entry(M)\
    jenum_entry(N)\
    jenum_entry(O)\
    jenum_entry(P)\
    jenum_entry(Q)
#include  <jlib/jenum.h>

TEST("enum hashmap, rehashing and iterate") {
    auto foo_name = hash_map<Foo, string>{};
    foo_name.insert(Foo::A, "A");
    foo_name.insert(Foo::B, "B");
    foo_name.insert(Foo::C, "C");
    foo_name.insert(Foo::D, "D");
    foo_name.insert(Foo::E, "E");
    foo_name.insert(Foo::F, "F");
    foo_name.insert(Foo::G, "G");
    foo_name.insert(Foo::H, "H");
    foo_name.insert(Foo::I, "I");
    foo_name.insert(Foo::J, "J");
    foo_name.insert(Foo::K, "K");
    foo_name.insert(Foo::L, "L");
    foo_name.insert(Foo::M, "M");
    foo_name.insert(Foo::N, "N");
    foo_name.insert(Foo::O, "O");
    foo_name.insert(Foo::P, "P");
    foo_name.insert(Foo::Q, "Q");

    for (const auto& [ k, v ]: foo_name) {
        ASSERT(foo_name[k] == v)
        log(to_string(k), v);
    }
}

TEST("iterate") {
    auto h = hash_map<string, double> {
        { "x", 1 },
        { "y", 2 },
    };

    for (const auto& [ k, v ]: h) {
        log(k, v);
    }
}