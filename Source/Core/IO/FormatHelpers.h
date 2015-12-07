#pragma once

#include "Core/Core.h"

#include "Core/Meta/StronglyTyped.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, SizeInBytes);
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, SizeInBytes value);
void Format(wchar_t *buffer, size_t capacity, SizeInBytes value);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const SizeInBytes& size) {
    _Char buffer[32];
    Format(buffer, lengthof(buffer), size);
    return oss << buffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, CountOfElements);
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, CountOfElements count);
void Format(wchar_t *buffer, size_t capacity, CountOfElements count);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const CountOfElements& count) {
    _Char buffer[32];
    Format(buffer, lengthof(buffer), count);
    return oss << buffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Count>
struct Repeater {
    const T *Value;
};
//----------------------------------------------------------------------------
template <size_t _Count, typename T>
Repeater<T, _Count> Repeat(const T& value) {
    return Repeater<T, _Count>{ &value };
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T, size_t _Count>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Repeater<T, _Count>& repeat) {
    for (size_t i = 0; i < _Count; ++i)
        oss << *repeat.Value;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
