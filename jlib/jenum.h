// there is no #pragma once or header guard by design as you might want to include it more than once

#if (!defined jenum_name || !defined jenum)
#error "Must #define jenum_main before including jenum"
#endif

#define jenum_entry(f) f,
enum class jenum_name : uint8_t { jenum() };
#undef jenum_entry

#define jenum_entry(f) { jenum_name::f, "Foo::"#f },
string to_string(jenum_name f) {
    static hash_map<jenum_name, string> _ = { jenum() };
    return _[f];
}
#undef jenum_entry

#undef jenum_name
#undef jenum_entry
