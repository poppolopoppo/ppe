#pragma once

#include <type_traits>

#include "Meta/Hash_fwd.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t... _Indices>
CONSTEXPR bool SequenceEquals(const T* lhs, const T* rhs, std::index_sequence<_Indices...>) {
    return (true/* handle empty sequences */ && lhs[_Indices] == rhs[_Indices] && ...);
}
//----------------------------------------------------------------------------
template <typename _Char>
struct TLiteralString {
    const _Char* Data;
    const size_t Size;

    CONSTEXPR TLiteralString() : TLiteralString(nullptr, 0) {}
    CONSTEXPR explicit TLiteralString(const _Char* data, size_t size) : Data(data), Size(size) {}

    template <size_t _Dim>
    CONSTEXPR TLiteralString(const _Char (&str)[_Dim])
        : TLiteralStirng(str, _Dim - 1/* remove trailing '\0' */)
    {}

    CONSTEXPR TLiteralString(const TLiteralString&) = default;
    CONSTEXPR TLiteralString& operator =(const TLiteralString&) = default;

    CONSTEPXR bool Equals(const TLiteralString& other) const {
        return (Size == other.Size && SequenceEquals(Data, other.Data, std::make_index_sequence<Size>{}));
    }

    inline friend CONSTEXPR bool operator ==(const TLiteralString& lhs, const TLiteralString& rhs) { return (lhs.Equals(rhs)); }
    inline friend CONSTEXPR bool operator !=(const TLiteralString& lhs, const TLiteralString& rhs) { return (not lhs.Equals(rhs)); }

    CONSTEPXR bool Less(const TLiteralString& other) const {
        forrange(i, 0, Min(Size, other.Size)) {
            if (Data[i] < other.Data[i])
                return true;
            else if (Data[i] > other.Data[i])
                return false;
        }
        return false;
    }

    inline friend CONSTEXPR bool operator < (const TLiteralString& lhs, const TLiteralString& rhs) { return (lhs.Less(rhs)); }
    inline friend CONSTEXPR bool operator >=(const TLiteralString& lhs, const TLiteralString& rhs) { return (not lhs.Less(rhs)); }

    CONSTEXPR size_t HashValue() const { return hash_sequence_constexpr(Data, std::make_index_sequence<Size>{}); }

    inline friend CONSTEXPR size_t hash_value(const TLiteralString& s) { return s.HashValue(); }
};
//----------------------------------------------------------------------------
using FLiteralString = TLiteralString<char>;
using FWLiteralString = TLiteralString<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
