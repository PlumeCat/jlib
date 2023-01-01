#include <string>
#include <jlib/hash_map.h>
#include <jlib/test_framework.h>

using namespace std;


TEST("hashmap") {
    auto h = hash_map<string, string>(32);

    h.insert("hello", "world");
    h.insert("foo", "bar");
    h.insert("baz", "quz");
    h.insert("this", "is");
    h.insert("hello", "world2");

    ASSERT(h.contains("foo"));
    ASSERT(h.contains("hello"));

    ASSERT(!h.empty());
    ASSERT(h.find("hello") != h.end());

    for (const auto& [ k, v ]: h) {
        log(k, v);
    }

}
