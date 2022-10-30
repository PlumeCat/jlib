#pragma once

template<typename T>
struct deferred {
    T func;
    deferred(T f) : func(f) {}
    ~deferred() { func(); }
};
struct defer_dummy {};
template<typename F> static deferred<F> operator<<(defer_dummy, F f) { return deferred<F>(f); }
#define PASTE(a, b) a##b
#define defer2(c) auto PASTE(deferred__, c) = defer_dummy() << [&]
#define defer defer2(__COUNTER__)
