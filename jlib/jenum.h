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

template<IsEnum e> struct jenum_s {};

#define jenum_helper(Name, ...)                                                                                \
    template<> struct jenum_s<Name> {                                                                          \
        using Base = std::underlying_type_t<Name>;                                                             \
        Base __VA_ARGS__; /* declare the arguments as members of type Base */                                  \
        inline jenum_s() {                                                                                     \
            std::memset(this, 0, sizeof(*this)); /* set all members to zero */                                 \
            __VA_ARGS__; /* restores members that had an initial value while no-op'ing the ones that didn't */ \
            for (auto i = 1; i < count(); i++) {                                                               \
                (*this)[i] = (*this)[i] ? (*this)[i] : (*this)[i - 1] + 1; /* correct the non-first zeroes */  \
            }                                                                                                  \
        }                                                                                                      \
        inline constexpr int count() {                                                                             \
            return sizeof(*this) / sizeof(Base);                                                               \
        }                                                                                                      \
        inline Base& operator[](size_t i) const {                                                              \
            return *(((Base*)(void*)this) + i);                                                                \
        } /* TODO: undefined behaviour, replace with bit_cast / memcpy */                                      \
    };

template<IsEnum E> E constexpr enum_from_chars(std::string_view str) {
    const auto sc = str.find("::");
    if (sc == std::string_view::npos) {
        return E {};
    }
    return E {};
}
#define jenum_ostream_rsh(Name, ...)                           \
    inline std::ostream& operator<<(std::ostream& o, Name n) { \
        return o << #Name << "::" << (int)n;                   \
    }
#define jenum_fromstring(Name, ...)

// #define disable_unused_warning()
// #define enable_unused_warning()
#if (defined __clang__)
#define disable_unused_warning() _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored\"-Wunused-value\"")
#define enable_unused_warning() _Pragma("clang diagnostic pop")
#else
#define disable_unused_warning() _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wunused-value\"")
#define enable_unused_warning() _Pragma("GCC diagnostic pop")
#endif

template<IsEnum E> constexpr std::string_view jenum_string() {
    // static_assert(false, "specialization only");
    return "";
}
#define jenum_stringdef(Name, ...)                                                             \
    static constexpr auto paste(jenum_stringhelper, Name) = std::string_view { #__VA_ARGS__ }; \
    template<> constexpr std::string_view jenum_string<Name>() {                               \
        return paste(jenum_stringhelper, Name);                                                \
    }

#define jenum(Name, ...)                                                                                                                        \
    disable_unused_warning();                                                                                                                   \
    enum Name { __VA_ARGS__ };                                                                                                                  \
    jenum_stringdef(Name, __VA_ARGS__) jenum_helper(Name, __VA_ARGS__) jenum_ostream_rsh(Name, __VA_ARGS__) jenum_fromstring(Name, __VA_ARGS__) \
        enable_unused_warning();

#define jenum_base(Name, Base, ...)                                                                                                             \
    disable_unused_warning();                                                                                                                   \
    enum Name : Base { __VA_ARGS__ };                                                                                                           \
    jenum_stringdef(Name, __VA_ARGS__) jenum_helper(Name, __VA_ARGS__) jenum_ostream_rsh(Name, __VA_ARGS__) jenum_fromstring(Name, __VA_ARGS__) \
        enable_unused_warning();

#define jenum_class(Name, ...)                                                                                                                  \
    disable_unused_warning();                                                                                                                   \
    enum class Name { __VA_ARGS__ };                                                                                                            \
    jenum_stringdef(Name, __VA_ARGS__) jenum_helper(Name, __VA_ARGS__) jenum_ostream_rsh(Name, __VA_ARGS__) jenum_fromstring(Name, __VA_ARGS__) \
        enable_unused_warning();

#define jenum_class_base(Name, Base, ...)                                                                                                       \
    disable_unused_warning();                                                                                                                   \
    enum class Name : Base { __VA_ARGS__ };                                                                                                     \
    jenum_stringdef(Name, __VA_ARGS__) jenum_helper(Name, __VA_ARGS__) jenum_ostream_rsh(Name, __VA_ARGS__) jenum_fromstring(Name, __VA_ARGS__) \
        enable_unused_warning();

// namespace jenum_internal {
//     template<typename T> consteval std::string_view templated_func_name() {
//         return std::source_location::current().function_name();
//     }
// };

// does a few static asserts to check everything will function as intended
// static_assert(std::string_view { "" } != std::source_location::current().function_name());
// consteval void compile_time_checks() {
//     // Check that std::source_location formatting is Clang/G++ style
//     static_assert(
//         templated_func_name<int>()
//         == "consteval std::string_view jenum_internal::templated_func_name() [with T = int; std::string_view = "
//            "std::basic_string_view<char>]"
//     );
// }
