#pragma once

#include <type_traits>

template<typename T> concept IsEnum = std::is_enum_v<T>;