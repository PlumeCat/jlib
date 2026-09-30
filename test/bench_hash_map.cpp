// bench_hash_map.cpp

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <thread>
#include <cmath>
#include <string_view>
#include <type_traits>
#include <unordered_map>
namespace chrono = std::chrono;
using namespace std::literals;

#include <jlib/test_framework.h>
#include <jlib/log.h>
#include <jlib/hash_map.h>

#ifdef __clang__
#include <cxxabi.h>
std::string demangle(const char* name) {
    int status = -4; // some arbitrary value to eliminate the compiler warning
    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };
    return (status==0) ? res.get() : name ;
}
#else
#define demangle
#endif


struct Bench {
    virtual void setup() {}
    virtual void func() {}
    virtual void teardown() {}
    virtual std::string_view name() { return "unknown benchmark"; }

    int trial() {
        setup();
        const auto before = chrono::steady_clock::now();
        func();
        const auto after = chrono::steady_clock::now();
        teardown();
        const auto diff = after - before;
        return chrono::duration_cast<chrono::microseconds>(diff).count();
    }
    void run(int num_trials = 1) {
        log("benching: ", name());
        auto trials = std::vector<int> {};
        auto total = uint64_t { 0 };
        auto a = INT32_MAX;
        auto b = 0;
        for (auto i = 0; i < num_trials; i++) {
            std::this_thread::sleep_for(chrono::milliseconds(10));
            auto t = trial();
            total += t;
            a = std::min(t, a);
            b = std::max(t, b);
            trials.emplace_back(t);
        }
        log(" - min:" , a);
        log(" - max:", b);
        log(" - average:", double(total)/ num_trials);
    }
};


// static const auto TESTSIZE = 100'000;
static const auto TESTSIZE = 1'000'000;

static auto KEYS_STRING = std::vector<std::string> {};
static auto KEYS_INT = std::vector<int> {};

void init_keys() {
    for (auto i = 0; i < TESTSIZE; i++) {
        KEYS_INT.emplace_back(rand());
        auto& s = KEYS_STRING.emplace_back(100, 'A');
        for (auto& c: s) { c = char('A' + rand() % 26); }
    }
}

template<typename Map>
struct Inserts final : public Bench {
    std::string name_ = "inserts "s + std::string { demangle(typeid(Map).name()) };
    virtual std::string_view name() override { return name_; }

    Map map;
    const std::vector<typename Map::key_type>& KEYS;

    Inserts(const auto& keys): KEYS(keys) {}
    virtual void setup() override {
        map = Map {};
        srand(12345);
    }
    virtual void func() override {
        for (auto i = 0; i < TESTSIZE; i++) {
            auto k = KEYS[rand() % KEYS.size()];
            map.insert_or_assign(k, k);
        }
    }
};

template<typename Map>
struct Reads final : public Bench {
    std::string name_ = "reads "s + std::string { demangle(typeid(Map).name()) };
    virtual std::string_view name() override { return name_; }

    Map map;
    const std::vector<typename Map::key_type>& KEYS;

    Reads(const auto& keys): KEYS(keys) {}
    virtual void setup() override {
        map = Map {};
        srand(23456);
        for (auto i = 0; i < TESTSIZE; i++) {
            auto k = KEYS[rand() % KEYS.size()];
            auto v = KEYS[rand() % KEYS.size()];
            map.insert_or_assign(k, v);
        }
    }

    virtual void func() override {
        for (auto i = 0; i < TESTSIZE; i++) {
            map.find(KEYS[rand() % KEYS.size()]);
        }
    }
};

template<typename Map>
struct Iterate final : public Bench {
    std::string name_ = "iteration "s + std::string { demangle(typeid(Map).name()) };
    virtual std::string_view name() override { return name_; }

    Map map;

    virtual void setup() override {
        map = Map {};
        srand(1214);
        for (auto i = 0; i < TESTSIZE; i++) {
            const auto k = rand();
            const auto v = rand();
            map.insert_or_assign(k, v);
        }
    }

    virtual void func() override {
        auto sum = 0;
        for (auto [ k, v ]: map) {
            sum += v;
        }
        errno += sum;
    }
};

TEST("init keys") {
    srand(12345);
    init_keys();
}

TEST("bench hash_map inserts") {
    log("inserts: ", TESTSIZE);
    Inserts<std::unordered_map<std::string, std::string>>{ KEYS_STRING }.run(20);
    Inserts<hash_map<std::string, std::string>>{ KEYS_STRING }.run(20);
    Inserts<std::unordered_map<int, int>>{ KEYS_INT }.run(20);
    Inserts<hash_map<int, int>>{ KEYS_INT }.run(20);
}

TEST("bench hash_map reads") {
    log("reads: ", TESTSIZE);
    Reads<std::unordered_map<std::string, std::string>>{KEYS_STRING}.run(20);
    Reads<hash_map<std::string, std::string>>{KEYS_STRING}.run(20);
    Reads<std::unordered_map<int, int>>{KEYS_INT}.run(20);
    Reads<hash_map<int, int>>{KEYS_INT}.run(20);
}

TEST("bench hash_map iteration") {
    Iterate<std::unordered_map<int, int>>().run(20);
    Iterate<hash_map<int, int>>().run(20);
}
