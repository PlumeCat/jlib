#include <jlib/hash_map.h>
#include <jlib/test_framework.h>

using namespace std;

struct RubbishHash {
    static uint64_t hash(const string& key) {
        return key.size();
    }
};

TEST("hashmap") {
    // auto h = hash_map<string, string, RubbishHash>(32);
    auto h = hash_map(16);
    h.set("hello", "world");
    h.set("foo", "bar");
    h.set("this", "is");
    h.set("four", "4");
    h.set("this", "this");
    h.set("baz", "quz");
    h.set("four", "41");

    ASSERT(h.get("foo") == "bar");
    ASSERT(h.get("baz") == "quz");
    ASSERT(h.get("hello") == "world");

    h.foreach([](auto k, auto v) { log<false>(' ', '\'', k, "' = '", v, '\''); });
}
