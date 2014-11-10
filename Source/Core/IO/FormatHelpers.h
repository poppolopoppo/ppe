#pragma once

#include "Core/Core.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct SizeInBytes {
    uint64_t Value;

    void Format(char *buffer, size_t capacity) const;
    void Format(wchar_t *buffer, size_t capacity) const;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const SizeInBytes& sizeInBytes) {
    _Char buffer[32];
    sizeInBytes.Format(buffer, lengthof(buffer));
    return oss << buffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CountOfElements {
    uint64_t Value;

    void Format(char *buffer, size_t capacity) const;
    void Format(wchar_t *buffer, size_t capacity) const;
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const CountOfElements& countOfElements) {
    _Char buffer[32];
    countOfElements.Format(buffer, lengthof(buffer));
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
