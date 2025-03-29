#include <string>

#include "jlib/generic_ostream.h"
#include "jlib/hash_map.h"
#include "jlib/test_framework.h"



template<typename K, typename V, typename... Args> std::vector<K> collect_keys(const hash_map<K, V>& map) {
    auto v = std::vector<K> {};
    for (auto& [ k, _ ]: map) {
        v.emplace_back(k);
    }
    std::sort(v.begin(), v.end());
    return v;
}

template<typename K, typename V, typename... Args> std::vector<V> collect_vals(const hash_map<K, V>& map) {
    auto v = std::vector<V> {};
    for (auto& [ _, x ]: map) {
        v.emplace_back(x);
    }
    std::sort(v.begin(), v.end());
    return v;
}


TEST("hashmap contains") {
    auto h = hash_map<std::string, std::string> {
        { "foo", "bar" },
        { "baz", "quz" },
        { "qux", "quux" }
    };
    ASSERT(h.contains("foo"));
    ASSERT(h.contains("baz"));
    ASSERT(h.contains("qux"));
    ASSERT(!h.contains("hello"));

    //      hash_map();
    //      h.begin;
    //      h.end;

    // h.clear;
    //      h.contains;
    // h.find;
    // h.at;

    // h.insert;
    //      h.operator=; // move/assign, moved from, etc
    // h.operator[]; // create new if not existing,
    //      h.remove;

    // h.size;
    // h.empty;
}

TEST("hashmap []") {
    auto h = hash_map<int, int> {};
    for (int i = 0; i < 3; i++) {
        if (h.contains(i)) {
            ASSERT(false);
        }
    }
}

TEST("hashmap remove") {
    auto h = hash_map<int, int> {
        { 0, 0 },
        { 1, 1 },
        { 2, 2 }
    };

    ASSERT(h.contains(0));
    ASSERT(h.contains(1));
    ASSERT(h.contains(2));
    ASSERT(h.size() == 3);

    ASSERT(h.erase(0) == 1);
    ASSERT(h.erase(2) == 1);

    ASSERT(!h.contains(0));
    ASSERT(!h.contains(2));
    ASSERT(h.contains(1));
    ASSERT(h.size() == 1);
}

TEST("hashmap clear") {
    auto h = hash_map<std::string, std::string> {
        { "foo", "bar" },
        { "baz", "quz" },
        { "qux", "quux" }
    };

    ASSERT(h.contains("foo"));
    ASSERT(h.contains("baz"));
    ASSERT(h.contains("qux"));
    ASSERT(h.size() == 3);
    h.clear();
    ASSERT(!h.contains("foo"));
    ASSERT(!h.contains("baz"));
    ASSERT(!h.contains("qux"));
    ASSERT(h.size() == 0);
}

TEST("hashmap copy/move") {
    auto h = hash_map<std::string, int> {
        { "one", 1 },
        { "two", 2 },
        { "four", 5 }
    };

    // copy ctor
    decltype(h) g(h);
    ASSERT(g.size() == h.size());
    ASSERT(g.contains("one") && g.contains("two")&& g.contains("four"));
    ASSERT(h.contains("one") && h.contains("two") && h.contains("four"));

    // copy assign
    g = h;
    ASSERT(g.size() == h.size());
    ASSERT(g.contains("one") && g.contains("two")&& g.contains("four"));
    ASSERT(h.contains("one") && h.contains("two") && h.contains("four"));

    // move ctor
    decltype(g) f(std::move(g));
    ASSERT(f.size() == 3);
    ASSERT(f.contains("one") && f.contains("two")&& f.contains("four"));
    ASSERT(g.size() == 0);
    ASSERT(!g.contains("one") && !g.contains("two") && !g.contains("four"));

    // move assign
    g = f;
    f = std::move(g);
    ASSERT(f.size() == 3);
    ASSERT(f.contains("one") && f.contains("two")&& f.contains("four"));
    ASSERT(g.size() == 0);
    ASSERT(!g.contains("one") && !g.contains("two") && !g.contains("four"));
}

TEST("hashmap iterate") {
    auto h = hash_map<std::string, int> {
        { "hello"s, 1 },
        { "world"s, 3 },
        { "this"s, 2 },
        { "is"s, 4 },
        { "a"s, 8 },
        { "hashmap"s, 6 }
    };

    auto keys = collect_keys(h);
    auto vals = collect_vals(h);

    ASSERT(keys == std::vector { "a"s, "hashmap"s, "hello"s, "is"s, "this"s, "world"s });
    ASSERT(vals == std::vector { 1, 2, 3, 4, 6, 8 });
}

TEST("integer hashmap") {
    auto h = hash_map<uint32_t, uint32_t> {
        { 0, 100 },
        { 1, 1 },
        { 2, 4 },
        { 3, 9 }
    };

    ASSERT(h.contains(1));
    ASSERT(h.erase(5) == 0);
    ASSERT_THROWS(h.at(10));
    ASSERT(h.at(0) == 100);
}

TEST("enum hashmap") {
    enum class Foo: uint64_t {
        Bar = 14,
        Baz = 1937,
        Quz = 32,
        Quux,
    };

    auto FooName = hash_map<Foo, std::string, default_hash> {
        { Foo::Bar,  "Foo.Bar"  },
        { Foo::Baz,  "Foo.Baz"  },
        { Foo::Quz,  "Foo.Quz"  },
        { Foo::Quux, "Foo.Quux" },
    };

    for (const auto& [k, v]: FooName) {
        ASSERT(FooName[k] == v);
    }
}