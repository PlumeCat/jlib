// bench_hash_map.cpp

#include <chrono>
#include <cstdint>
#include <thread>
#include <cmath>
#include <string_view>
#include <unordered_map>
namespace chrono = std::chrono;
using namespace std::literals;

#include <jlib/test_framework.h>
#include <jlib/log.h>
#include <jlib/hash_map.h>


template<typename T> constexpr std::string_view ezname() {
    // log(__FUNCTION__);
    constexpr auto fn = std::string_view { __PRETTY_FUNCTION__ };
    constexpr auto A = fn.find("[T = ") + 5;
    constexpr auto B = fn.find("]", A);
    // log(fn, A, B, fn.substr(A, B-A));
    return(fn.substr(A, B-A));

}
static_assert(ezname<int>() == "int"sv);
static_assert(ezname<std::vector<char>>() == "std::vector<char>"sv);

struct Bench {
    virtual void setup() {}
    virtual void func() {}
    virtual void teardown() {}
    virtual std::string_view name() {
        return "unknown benchmark";
    }

    int trial() {
        setup();
        const auto before = chrono::steady_clock::now();
        func();
        const auto after = chrono::steady_clock::now();
        teardown();
        const auto diff = after - before;
        return chrono::duration_cast<chrono::microseconds>(diff).count();
    }
    void run(int n = 1) {
        log("benching: ", name());
        auto trials = std::vector<int> {};
        auto total = uint64_t { 0 };
        auto a = INT32_MAX;
        auto b = 0;
        for (auto i = 0; i < n; i++) {
            std::this_thread::sleep_for(chrono::milliseconds(10));
            auto t = trial();
            total += t;
            a = std::min(t, a);
            b = std::max(t, b);
            trials.emplace_back(t);
        }
        log(" - min:" , a);
        log(" - max:", b);
        log(" - average:", double(total) / n);
    }
};


static const auto N = 100'000;


#define STRINGBENCH
#ifdef STRINGBENCH
using TestType = std::string;
static auto S() {
    return std::string(100, char('A' + rand() % 26));
}
#else
using TestType = int;
static auto S() {
    return rand();
}
#endif
using UM = std::unordered_map<TestType, TestType>;
using HM = hash_map<TestType, TestType>;
static auto KEYS = std::vector<TestType> {};
void init_keys() {
    if (KEYS.size()) {
        return;
    }
    for (auto i = 0; i < N; i++) {
        KEYS.emplace_back(S());
    }
}



template<typename Map>
struct Inserts final : public Bench {
    std::string name_ = "10k inserts "s + std::string { ezname<Map>() };
    virtual std::string_view name() override { return name_; }

    Map map;

    virtual void setup() override {
        map = Map {};
        srand(12345);
        init_keys();
    }
    virtual void func() override {
        for (auto i = 0; i < N; i++) {
            auto k = KEYS[rand() % KEYS.size()];
            map.insert_or_assign(k, k);
        }
    }
};

template<typename Map>
struct Reads final : public Bench {
    std::string name_ = "10k reads "s + std::string { ezname<Map>() };
    virtual std::string_view name() override { return name_; }

    Map map;

    virtual void setup() override {
        map = Map {};
        srand(23456);
        init_keys();
        for (auto i = 0; i < N; i++) {
            auto k = KEYS[rand() % KEYS.size()];
            auto v = KEYS[rand() % KEYS.size()];
            map.insert_or_assign(k, v);
        }
    }

    virtual void func() override {
        auto found = 0;
        for (auto i = 0; i < N; i++) {
            if (map.contains(KEYS[rand() % KEYS.size()])) {
                found++;
            }
        }
    }
};

TEST("bench hash_map 10k inserts") {
    Inserts<HM>{}.run(20);
    Inserts<UM>{}.run(20);
}

TEST("bench hash_map 10k reads") {
    Reads<HM>{}.run(20);
    Reads<UM>{}.run(20);
}
