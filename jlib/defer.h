#pragma once

template<typename T>
struct deferred {
    T func;
    deferred(T f) : func(f) {}
    ~deferred() { func(); }
};
struct defer_dummy {};
template<typename F> deferred<F> operator<<(defer_dummy, F f) { return deferred<F>(f); }
#ifndef paste
#define paste(a, b) a##b
#endif
#define _defer(c) auto paste(_deferred, c) = defer_dummy{} << [&]
#define defer _defer(__COUNTER__)

template<typename T>
struct deferred_cancelable {
    T func;
    bool cancel_;
    deferred_cancelable(T f): func(f), cancel_(false) {}
    ~deferred_cancelable() { if (!cancel_) func(); }
    void cancel() { cancel_ = true; }
};
struct defer_dummy_c {};
template<typename F> deferred_cancelable<F> operator<<(defer_dummy_c, F f) { return deferred_cancelable<F>(f); }
#define defer_cancelable defer_dummy_c{} << [&]

#undef

/*
Usage:

    void f() {
        auto f = fopen("...");

        defer { fclose(f); }
    }
*/
