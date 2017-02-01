#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryView.h"
#include "Core/Meta/StronglyTyped.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, FSizeInBytes);
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, FSizeInBytes value);
void Format(wchar_t *buffer, size_t capacity, FSizeInBytes value);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const FSizeInBytes& size) {
    _Char buffer[32];
    Format(buffer, lengthof(buffer), size);
    return oss << buffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_STRONGLYTYPED_NUMERIC_DEF(uint64_t, FCountOfElements);
//----------------------------------------------------------------------------
void Format(char *buffer, size_t capacity, FCountOfElements count);
void Format(wchar_t *buffer, size_t capacity, FCountOfElements count);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const FCountOfElements& count) {
    _Char buffer[32];
    Format(buffer, lengthof(buffer), count);
    return oss << buffer;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Count>
struct TRepeater {
    const T *Value;
};
//----------------------------------------------------------------------------
template <size_t _Count, typename T>
TRepeater<T, _Count> Repeat(const T& value) {
    return TRepeater<T, _Count>{ &value };
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T, size_t _Count>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const TRepeater<T, _Count>& repeat) {
    for (size_t i = 0; i < _Count; ++i)
        oss << *repeat.Value;
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Elt, typename _Sep>
struct TJoin {
    TMemoryView<_Elt> Data;
    _Sep Separator;
    TJoin(const TMemoryView<_Elt>& data, _Sep separator)
        : Data(data), Separator(separator) {}
};
//----------------------------------------------------------------------------
template <typename T>
TJoin<T, const char*> CommaSeparated(const TMemoryView<T>& data) {
    return TJoin<T, const char*>(data, ", ");
};
//----------------------------------------------------------------------------
template <typename _Char, typename _Elt, typename _Sep>
std::basic_ostream<_Char>& operator <<(std::basic_ostream<_Char>& oss, const TJoin<_Elt, _Sep>& join) {
    if (join.Data.size()) {
        oss << join.Data.front();
        forrange(i, 1, join.Data.size())
            oss << join.Separator << join.Data[i];
    }
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FHexDump {
    TMemoryView<const u8> RawData;
    size_t BytesPerRow;
    FHexDump(const TMemoryView<const u8>& rawData, size_t bytesPerRow = 16)
        : RawData(rawData), BytesPerRow(bytesPerRow) {}
};
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const FHexDump& hexDump);
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const FHexDump& hexDump);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Replaces std::endl which will cause almost always unneeded/unwanted flush of the stream !
template<class _Elem, class _Traits>
std::basic_ostream<_Elem, _Traits>& eol(std::basic_ostream<_Elem, _Traits>& oss) {
    oss.put(oss.widen('\n'));
    return (oss);
}
//----------------------------------------------------------------------------
// Same than eol but with a CRLF
template<class _Elem, class _Traits>
std::basic_ostream<_Elem, _Traits>& crlf(std::basic_ostream<_Elem, _Traits>& oss) {
    oss.put(oss.widen('\r'));
    oss.put(oss.widen('\n'));
    return (oss);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
