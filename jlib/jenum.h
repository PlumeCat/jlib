#pragma once

/*
TODO: rewrite with the __PRETTY_FUNCTION__ hack from magic_enum
*/

#include <cstring> // for memset
#include <type_traits> // for underlying_type_t
#define jenum_s(EnumName, ...) \
    template<typename T, typename Base = std::underlying_type_t<T>> struct paste(jenum_s, EnumName) {\
        using Type = paste(jenum_s, EnumName);\
        Base __VA_ARGS__; /* declare the arguments as members of type Base */\
        paste(jenum_s, EnumName)() {\
            std::memset(this, 0, sizeof(*this)); /* set all members to zero */\
            __VA_ARGS__; }/* restores members that had an initial value while no-op'ing the ones that didn't */\
    }; paste(jenum_s, EnumName)<EnumName> paste(jenum_s_, EnumName);

template<typename E> requires std::is_enum_v<E> E from_chars(const char* str) { return E{}; }

#define jenum_tostring(Name, ...) inline std::ostream& operator<<(std::ostream& o, Name n) {\
return o;\
    /*return ""; o << #Name "::" << e2s.get_or(n, "???");*/\
}
#define jenum_fromstring(Name, ...) template<> inline Name from_chars(const char* str) {\
    return Name{};\
}

#define jenum(Name, ...) \
    enum Name { __VA_ARGS__ }; \
    jenum_s(Name, __VA_ARGS__); \
    jenum_fromstring(Name, __VA_ARGS__); \
    jenum_tostring(Name, __VA_ARGS__);
#define jenum_class(Name, ...) \
    enum class Name { __VA_ARGS__ }; \
    jenum_s(Name, __VA_ARGS__); \
    jenum_fromstring(Name, __VA_ARGS__); \
    jenum_tostring(Name, __VA_ARGS__);
#define jenum_class_base(Name, Base, ...) \
    enum class Name: Base { __VA_ARGS__ }; \
    jenum_s(Name, __VA_ARGS__); \
    jenum_fromstring(Name, __VA_ARGS__); \
    jenum_tostring(Name, __VA_ARGS__);
