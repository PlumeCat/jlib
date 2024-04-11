#pragma once

/*
inspired by the magic_enum __PRETTY_FUNCTION__ approach,
but uses std::source_location instead
does some compile-time checks to make sure that works as expected
*/

#include <cstring> // memset
#include <ostream> // ostream
#include <source_location>
#include <string_view>
#include <type_traits>

#ifndef paste
#define paste(a, b) a##b
#endif
template<typename E> concept IsEnum = std::is_enum<E>::value;
#define jenum_s(Name, ...)                                                                                                            \
    struct paste(JENUM_VALUES_, Name) {                                                                                               \
        using Base = std::underlying_type_t<Name>;                                                                                    \
        Base __VA_ARGS__; /* declare the arguments as members of type Base */                                                         \
        paste(JENUM_VALUES_, Name)() {                                                                                                \
            std::memset(this, 0, sizeof(*this)); /* set all members to zero */                                                        \
            __VA_ARGS__; /* restores members that had an initial value while no-op'ing the ones that didn't */                        \
            for (auto i = sizeof(Base); i < sizeof(*this); i += sizeof(Base)) {                                                       \
                (*this)[i] = (*this)[i] ? (*this)[i] : (*this)[i - 1] + 1; /* correct the non-first zeroes */                         \
            }                                                                                                                         \
        }                                                                                                                             \
        Base& operator[](size_t i) {                                                                                                  \
            return ((Base*)this)[i];                                                                                                  \
        } /* TODO: undefined behaviour, replace with bit_cast / memcpy */                                                             \
    };

template<IsEnum E> E enum_from_chars(const char* str) {
    return E {};
}
#define jenum_tostring(Name, ...)                                                                                                     \
    inline std::ostream& operator<<(std::ostream& o, Name n) {                                                                        \
        return o << #Name << "::" << (int)n;                                                                                          \
    }
#define jenum_fromstring(Name, ...)

#if (defined __clang__)
#define disable_unused_warning() _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored\"-Wunused-value\"")
#define enable_unused_warning() _Pragma("clang diagnostic pop")
#else
#define disable_unused_warning() _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wunused-value\"")
#define enable_unused_warning() _Pragma("GCC diagnostic pop")
#endif

#define jenum(Name, ...)                                                                                                              \
    disable_unused_warning() enum Name { __VA_ARGS__ };                                                                               \
    jenum_s(Name, __VA_ARGS__) jenum_tostring(Name, __VA_ARGS__) jenum_fromstring(Name, __VA_ARGS__) enable_unused_warning()

#define jenum_base(Name, Base, ...)                                                                                                   \
    disable_unused_warning() enum Name : Base { __VA_ARGS__ };                                                                        \
    jenum_s(Name, __VA_ARGS__;\
    jenum_tostring(Name, __VA_ARGS__)\
    jenum_fromstring(Name, __VA_ARGS__)\
    enable_unused_warning()

#define jenum_class(Name, ...)                                                                                                        \
    disable_unused_warning() enum class Name { __VA_ARGS__ };                                                                         \
    jenum_s(Name, __VA_ARGS__) jenum_tostring(Name, __VA_ARGS__) jenum_fromstring(Name, __VA_ARGS__) enable_unused_warning()

#define jenum_class_base(Name, Base, ...)                                                                                             \
    disable_unused_warning() enum class Name : Base { __VA_ARGS__ };                                                                  \
    jenum_s(Name, __VA_ARGS__) jenum_tostring(Name, __VA_ARGS__) jenum_fromstring(Name, __VA_ARGS__) enable_unused_warning()

namespace jenum_internal {
template<typename T> consteval std::string_view templated_func_name() {
    return std::source_location::current().function_name();
}

// does a few static asserts to check everything will function as intended
// static_assert(std::string_view { "" } != std::source_location::current().function_name());
consteval void compile_time_checks() {
    // Check that std::source_location formatting is Clang/G++ style
    static_assert(
        templated_func_name<int>()
        == "consteval std::string_view jenum_internal::templated_func_name() [with T = int; std::string_view = "
           "std::basic_string_view<char>]"
    );
}
};
