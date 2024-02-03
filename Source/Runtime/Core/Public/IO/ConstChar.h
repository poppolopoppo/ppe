#pragma once

#include "Core_fwd.h"

#include "Meta/hash_fwd.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicConstChar {
    const _Char* Data = nullptr;

    TBasicConstChar() = default;

    CONSTEXPR TBasicConstChar(const _Char* data) NOEXCEPT : Data(data) {}

    CONSTEXPR const _Char* c_str() const NOEXCEPT { return Data; }
    CONSTEXPR operator const _Char* () const NOEXCEPT { return Data; }

    CONSTEXPR size_t length() const {
        if (nullptr == Data) return 0;
        size_t len = 0;
        for(const _Char* s = Data; s[len]; ++len);
        return len;
    }

    CONSTEXPR bool unsafe_length(size_t maxLength, size_t* outLength) const {
        *outLength = 0;
        if (nullptr == Data) return true;
        size_t len = 0;
        for(const _Char* s = Data; s[len]; ++len) {
            if (len > maxLength)
                return false;
        }
        *outLength = len;
        return true;
    }

    CONSTEXPR bool Equals(const TBasicConstChar& other) const NOEXCEPT {
        const _Char* a = Data;
        const _Char* b = other.Data;

        if (not (!!a & !!b))
            return (a == b);

        for (; *a & *b; ++a, ++b)
            if (*a != *b)
                return false;

        return (not (*a | *b));
    }

    CONSTEXPR bool EqualsI(const TBasicConstChar& other) const NOEXCEPT {
        const _Char* a = Data;
        const _Char* b = other.Data;

        if (not (!!a & !!b))
            return (a == b);

        for (; *a & *b; ++a, ++b)
            if (ToLower(*a) != ToLower(*b))
                return false;

        return (not (*a | *b));
    }

    CONSTEXPR bool Less(const TBasicConstChar& other) const NOEXCEPT {
        const _Char* a = Data;
        const _Char* b = other.Data;

        if (not (!!a & !!b))
            return (!a & !!b);

        for (; *a & *b; ++a, ++b)
            if (*a >= *b)
                return false;

        return (!(*a | *b) | ((!*a) & *b));
    }

    CONSTEXPR bool LessI(const TBasicConstChar& other) const NOEXCEPT {
        const _Char* a = Data;
        const _Char* b = other.Data;

        if (not (!!a & !!b))
            return (!a & !!b);

        for (; *a & *b; ++a, ++b)
            if (ToLower(*a) >= ToLower(*b))
                return false;

        return ( !(*a | *b) | ((!*a) & *b) );
    }

    CONSTEXPR size_t HashValue() const NOEXCEPT {
        size_t h = PPE_HASH_VALUE_SEED;
        for (const _Char* ch = Data; *ch; ++ch)
            h = hash_size_t_constexpr(h, *ch);
        return h;
    }

    CONSTEXPR size_t HashValueI() const NOEXCEPT {
        size_t h = PPE_HASH_VALUE_SEED;
        for (const _Char* ch = Data; *ch; ++ch)
            h = hash_size_t_constexpr(h, ToLower(*ch));
        return h;
    }

    CONSTEXPR void Swap(TBasicConstChar& other) NOEXCEPT {
        const _Char* const tmp = Data;
        Data = other.Data;
        other.Data = tmp;
    }

    CONSTEXPR TBasicStringView<_Char> MakeView() const NOEXCEPT {
        return TBasicStringView<_Char>(Data, Data ? length() : 0);
    }

    CONSTEXPR inline friend bool operator ==(const TBasicConstChar& lhs, const TBasicConstChar& rhs) NOEXCEPT { return lhs.Equals(rhs); }
    CONSTEXPR inline friend bool operator !=(const TBasicConstChar& lhs, const TBasicConstChar& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    CONSTEXPR inline friend bool operator < (const TBasicConstChar& lhs, const TBasicConstChar& rhs) NOEXCEPT { return lhs.Less(rhs); }
    CONSTEXPR inline friend bool operator >=(const TBasicConstChar& lhs, const TBasicConstChar& rhs) NOEXCEPT { return not operator < (lhs, rhs); }

    CONSTEXPR inline friend hash_t hash_value(const TBasicConstChar& s) NOEXCEPT { return s.HashValue(); }

    CONSTEXPR inline friend void swap(TBasicConstChar& lhs, TBasicConstChar& rhs) NOEXCEPT { lhs.Swap(rhs); }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TConstCharEqualTo {
    CONSTEXPR bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicConstChar<_Char>& rhs) const NOEXCEPT {
        return (lhs.Data == rhs.Data || lhs.Equals(rhs));
    }
};
template <typename _Char>
struct TConstCharEqualTo<_Char, ECase::Insensitive> {
    CONSTEXPR bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicConstChar<_Char>& rhs) const NOEXCEPT {
        return (lhs.Data == rhs.Data || lhs.EqualsI(rhs));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TConstCharLess {
    CONSTEXPR bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicConstChar<_Char>& rhs) const NOEXCEPT {
        return (lhs.Data != rhs.Data && lhs.Less(rhs));
    }
};
template <typename _Char>
struct TConstCharLess<_Char, ECase::Insensitive> {
    CONSTEXPR bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicConstChar<_Char>& rhs) const NOEXCEPT {
        return (lhs.Data != rhs.Data && lhs.LessI(rhs));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TConstCharHasher {
    CONSTEXPR hash_t operator ()(const TBasicConstChar<_Char>& cstr) const NOEXCEPT {
        return cstr.HashValue();
    }
};
template <typename _Char>
struct TConstCharHasher<_Char, ECase::Insensitive> {
    CONSTEXPR hash_t operator ()(const TBasicConstChar<_Char>& cstr) const NOEXCEPT {
        return cstr.HashValueI();
    }
};
//----------------------------------------------------------------------------
template <typename _CharA, typename _CharB>
TBasicTextWriter<_CharA>& operator <<(TBasicTextWriter<_CharA>& oss, const TBasicConstChar<_CharB>& cstr) {
    return oss << cstr.MakeView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
