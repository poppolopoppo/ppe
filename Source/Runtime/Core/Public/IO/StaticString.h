#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformString.h"
#include "IO/ConstChar.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

#include <algorithm>

#define PPE_STATICSTRING_CAPACITY 512

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = PPE_STATICSTRING_CAPACITY>
struct TBasicStaticString {
    size_t Len{ 0 };
    _Char Data[_Capacity]{ 0 };

    TBasicStaticString() = default;

    TBasicStaticString(const TBasicStaticString&) = default;
    TBasicStaticString& operator =(const TBasicStaticString&) = default;

    CONSTEXPR TBasicStaticString(size_t len, _Char broadcast = Zero) {
        Assign(len, broadcast);
    }

    CONSTEXPR TBasicStaticString(const TBasicStringView<_Char>& str) {
        Assign(str);
    }
    CONSTEXPR TBasicStaticString& operator =(const TBasicStringView<_Char>& str) {
        Assign(str);
        return (*this);
    }

    CONSTEXPR TBasicStaticString(const TBasicConstChar<_Char>& cstr) {
        Assign(cstr.MakeView());
    }
    CONSTEXPR TBasicStaticString& operator =(const TBasicConstChar<_Char>& cstr) {
        Assign(cstr.MakeView());
        return (*this);
    }

    template <size_t _Len, class = Meta::TEnableIf<_Len <= _Capacity>>
    CONSTEXPR TBasicStaticString(const _Char (&arr)[_Len]) {
        Assign(MakeStringView(arr));
    }
    template <size_t _Len, class = Meta::TEnableIf<_Len <= _Capacity>>
    CONSTEXPR TBasicStaticString& operator =(const _Char (&arr)[_Len]) {
        Assign(MakeStringView(arr));
        return (*this);
    }

    CONSTEXPR CONSTF bool empty() const { return (0 == Len); }
    CONSTEXPR CONSTF size_t size() const { return Len; }
    CONSTEXPR CONSTF size_t capacity() const { return _Capacity; }

    CONSTEXPR CONSTF TMemoryView<_Char> Buf() { return MakeView(Data); }
    CONSTEXPR CONSTF auto Str() const { return TBasicStringView<_Char>(Data, Len); }
    CONSTEXPR CONSTF auto c_str() const { return TBasicConstChar<_Char>{ NullTerminated() }; }

    CONSTEXPR operator _Char* () { return NullTerminated(); }
    CONSTEXPR CONSTF operator const _Char* () const { return NullTerminated(); }
    CONSTEXPR CONSTF operator TBasicStringView<_Char> () const { return Str(); }
    CONSTEXPR CONSTF operator TBasicConstChar<_Char> () const { return c_str(); }

    CONSTEXPR _Char* NullTerminated() {
        Assert(capacity() >= Len + 1);
        Data[Len] = Zero;
        return Data;
    }
    CONSTEXPR const _Char* NullTerminated() const {
        Assert(capacity() >= Len + 1);
        Assert(Data[Len] == Zero);
        return Data;
    }

    CONSTEXPR void Assign(const TBasicStringView<_Char>& str) {
        Len = str.size();
        if (Len >= _Capacity)
            Len = _Capacity - 1;
        forrange(i, 0, Len)
            Data[i] = str.data()[i]; // constexpr
        Data[Len] = Zero; // null-terminated
    }

    CONSTEXPR void Assign(size_t len, _Char broadcast = Zero) {
        Len = len;
        if (Len >= _Capacity)
            Len = _Capacity - 1;
        forrange(p, Data, Data + Len)
            *p = broadcast; // constexpr
        Data[Len] = Zero; // null-terminated
    }

    CONSTEXPR CONSTF bool Equals(const TBasicStaticString& other) const {
        if (Len != other.Len)
            return false;
        forrange(i, 0, Len) {
            if (Data[i] != other.Data[i])
                return false; // constexpr
        }
        return true;
    }

    CONSTEXPR CONSTF bool Less(const TBasicStaticString& other) const {
        forrange(i, 0, Min(Len, other.Len)) {
            if (Data[i] != other.Data[i])
                return (Data[i] < other.Data[i]);
        }
        return false;
    }

    CONSTEXPR void Clear() {
        Len = 0;
        Data[0] = Zero;
    }

    CONSTEXPR void Resize(size_t len, bool keepData = true) {
        Assert(len < _Capacity);
        Len = len;
        if (not keepData) {
            forrange(p, Data, Data + Len)
                *p = Zero;
        }
        Data[Len] = Zero;
    }

    CONSTEXPR CONSTF size_t SubstringOffset(const TBasicStringView<_Char>& str) const {
        size_t off = 0;
        for (; off + str.size() <= Len; ++off) {
            size_t subl = 0;
            for (; subl < str.size(); ++subl) {
                if (str[subl] == Data[off + subl])
                    break;
            }
            if (subl == str.size())
                return off;
        }
        return Len;
    }

    CONSTEXPR bool operator ==(const TBasicStaticString& other) const { return Equals(other); }
    CONSTEXPR bool operator !=(const TBasicStaticString& other) const { return (not operator ==(other)); }

    CONSTEXPR bool operator < (const TBasicStaticString& other) const { return Less(other); }
    CONSTEXPR bool operator >=(const TBasicStaticString& other) const { return (not operator < (other)); }

    CONSTEXPR bool operator > (const TBasicStaticString& other) const { return other.Less(*this); }
    CONSTEXPR bool operator <=(const TBasicStaticString& other) const { return (not operator > (other)); }

    friend CONSTF hash_t hash_value(const TBasicStaticString& value) {
        return hash_string(value.Str());
    }

    friend TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TBasicStaticString& str) {
        return oss << str.Str();
    }

    CONSTEXPR CONSTF size_t lengthof(const TBasicStaticString& s) {
        return s.size();
    }
};
//----------------------------------------------------------------------------
template <size_t _Capacity = PPE_STATICSTRING_CAPACITY>
using TStaticString = TBasicStaticString<char, _Capacity>;
template <size_t _Capacity = PPE_STATICSTRING_CAPACITY>
using TWStaticString = TBasicStaticString<wchar_t, _Capacity>;
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto Lower(const TBasicStringView<_Char>& src) {
    TBasicStaticString<_Char, _Capacity> dst;
    Assert(dst.capacity() >= src.size() + 1);
    dst.Len = src.size();
    ToLower(dst.Str(), src);
    return dst;
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto Lower(TBasicStaticString<_Char, _Capacity>&& src) {
    InplaceToLower(src.Str());
    return src;
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto Upper(const TBasicStringView<_Char>& src) {
    TBasicStaticString<_Char, _Capacity> dst;
    Assert(dst.capacity() >= src.size() + 1);
    dst.Len = src.size();
    ToUpper(src.Str(), src);
    return dst;
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto Upper(TBasicStaticString<_Char, _Capacity>&& src) {
    InplaceToUpper(src.Str());
    return src;
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto NullTerminated(const TBasicStringView<_Char>& src) {
    TBasicStaticString<_Char, _Capacity> dst;
    Assert(dst.capacity() >= src.size() + 1);
    src.CopyTo(dst.Buf());
    dst.Len = src.size();
    dst[src.size()] = _Char(0);
    return dst;
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto NullTerminated(TBasicStaticString<_Char, _Capacity>&& src) {
    Assert(src.capacity() >= src.size() + 1);
    src.NullTerminated();
    return src;
}
//----------------------------------------------------------------------------
template <size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto UTF_8_TO_WCHAR(const FStringView& src) {
    TWStaticString<_Capacity> wdst;
    wdst.Len = (FPlatformString::CHAR_to_WCHAR(ECodePage::UTF_8,
        wdst.Data, wdst.capacity(), src.data(), src.size()) - 1 );
    return wdst;
}
//----------------------------------------------------------------------------
template <size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto UTF_8_TO_WCHAR(const char* src) {
    return UTF_8_TO_WCHAR<_Capacity>(MakeCStringView(src));
}
//----------------------------------------------------------------------------
template <size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto WCHAR_TO_UTF_8(const FWStringView& wsrc) {
    TStaticString<_Capacity> dst;
    dst.Len = (FPlatformString::WCHAR_to_CHAR(ECodePage::ACP,
        dst.Data, dst.capacity(), wsrc.data(), wsrc.size()) - 1 );
    return dst;
}
//----------------------------------------------------------------------------
template <size_t _Capacity = PPE_STATICSTRING_CAPACITY>
auto WCHAR_TO_UTF_8(const wchar_t* wsrc) {
    return WCHAR_TO_UTF_8<_Capacity>(MakeCStringView(wsrc));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
