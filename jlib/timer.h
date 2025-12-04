#pragma once

#include <chrono>
using namespace std::literals;

template<
    typename clock_type=std::chrono::steady_clock,
    typename delta_type=std::chrono::microseconds,
    typename timepoint=decltype(clock_type::now())
> class Timer {
public:
    Timer(): last(clock_type::now()) {}
    Timer(const Timer&) = default;
    Timer(Timer&&) = default;
    Timer& operator=(const Timer&) = default;
    Timer& operator=(Timer&&) = default;

    uint64_t lap() {
        const auto next = clock_type::now();
        const auto diff = std::chrono::duration_cast<delta_type>(next - last).count();
        last = next;
        return diff;
    }
    void reset() {
        last = clock_type::now();
    }

private:
    timepoint last;
};
