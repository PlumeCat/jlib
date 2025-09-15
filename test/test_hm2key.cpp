// test_hm2key.cpp

// test_hashmap2.cpp
#include <unordered_map>

#include <jlib/test_framework.h>
#include <jlib/log.h>
#include <jlib/hash_map2.h>


int CTOR_LVAL = 0;
int CTOR_RVAL = 0;
int CTOR_COPY = 0;
int CTOR_MOVE = 0;
int ASSIGN_COPY = 0;
int ASSIGN_MOVE = 0;
int DTOR = 0;

struct DS {
    std::string s;

    DS(): s() {}
    DS(const std::string& s): s(s) { CTOR_LVAL++; }
    DS(std::string&& s): s(std::move(s)) { CTOR_RVAL++; }
    DS(const DS& d): s(d.s) { CTOR_COPY++; }
    DS(DS&& d): s(std::move(d.s)) { CTOR_MOVE++; }
    DS& operator=(const DS& d) { s = d.s; ASSIGN_COPY++; return *this; }
    DS& operator=(DS&& d) { s = std::move(d.s); ASSIGN_MOVE++; return *this; }
    ~DS() { DTOR++; }
};

bool operator==(const DS& l, const DS& r) { return l.s == r.s; }
bool operator==(const DS& l, const std::string& r) { return l.s == r; }
bool operator==(const std::string& l, const DS& r) { return l == r.s; }
namespace std {
    template<> struct hash<DS> {
        size_t operator()(const DS& ds) const { return std::hash<std::string>{}(ds.s); }
    };
};

#define COUNT_UM
#ifdef COUNT_UM
using Map = std::unordered_map<DS, DS>;
// #define first first
#else
using Map = hashmap<DS, DS>;
#define first key // HACK:
#endif

// random string generator
static auto S() {
    return std::string(10, char('A' + rand() % 26));
}

TEST("hashmap2 key bench") {
    srand(5550123);

    auto map = Map {};

    // bunch of random actions
    for (auto i = 0; i < 1000; i++) {
        switch (rand() % 3) {
        case 0: {
                // add random element
                auto k = S() + std::to_string(i);
                auto v = S();
                map.insert_or_assign(k, v);
                map.insert_or_assign(k, v);
            }
            break;
        case 1:

            // delete random element
            if (map.size() > 24) {
                auto n = rand() % map.size();
                auto e = map.begin(); for (auto i = 0; i < n; i++) e++;
                auto k = e->first;
                map.erase(k);
            }
            break;
        case 2:
            // update random element
            if (map.size()) {
                auto n = rand() % map.size();
                auto e = map.begin(); for (auto i = 0; i < n; i++) e++;
                auto k = e->first;
                auto v = S();
                map.insert_or_assign(k, v);
            }
            break;
        }
    }

    log<true, false>("Counts: ");
    log<true, false>(" - ctor_lval:", CTOR_LVAL);
    log<true, false>(" - ctor_rval:", CTOR_RVAL);
    log<true, false>(" - ctor_copy:", CTOR_COPY);
    log<true, false>(" - ctor_move:", CTOR_MOVE);
    log<true, false>(" - assign_copy:", ASSIGN_COPY);
    log<true, false>(" - assign_move:", ASSIGN_MOVE);
    log<true, false>(" - dtor:", DTOR);
};

