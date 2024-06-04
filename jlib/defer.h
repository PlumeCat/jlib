#pragma once

// Limitations: Can't throw exceptions from a deferred block, as it is implemented with destructors

template <typename T> struct deferred {
    T func;
    bool cancel_;
    deferred(T f): func(f), cancel_(false) {}
    ~deferred() {
        if (!cancel_) {
            func();
        }
    }
    void cancel() {
        cancel_ = true;
    }
};
struct defer_dummy {};
template <typename F> deferred<F> operator<<(defer_dummy, F f) {
    return deferred<F>(f);
}
#ifndef paste
#define paste(a, b) a##b
#endif
#define _defer(c) auto paste(_deferred, c) = defer_dummy {} << [&]
#define defer _defer(__COUNTER__)
#define defer_cancelable defer_dummy {} << [&]

/*
Contrived usage examples:

    // Guaranteed call
    void f() {
        auto f = fopen("...");
        defer { fclose(f); }
    }

    FILE* g() {
        auto f = fopen("...");
        auto d = defer { fclose(f); }

        if (read(f) != MAGIC_NUMBER) {
            throw "invalid file";
        }

        d.cancel();
        return f;
    }
*/
