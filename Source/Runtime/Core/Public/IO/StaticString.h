#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformString.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

#define PPE_STATICSTRING_CAPACITY 512

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = PPE_STATICSTRING_CAPACITY>
struct TBasicStaticString {
    size_t Len{ 0 };
    _Char Data[_Capacity];

    bool empty() const { return (0 == Len); }
    size_t size() const { return Len; }
    CONSTEXPR size_t capacity() const { return _Capacity; }

    TMemoryView<_Char> Buf() { return MakeView(Data); }
    auto Str() const { return TBasicStringView<_Char>(Data, Len); }

    operator _Char* () { return NullTerminated(); }
    operator const _Char* () const { return NullTerminated(); }
    operator TBasicStringView<_Char> () const { return Str(); }

    char* NullTerminated() {
        Assert(capacity() >= Len + 1);
        Data[Len] = _Char(0);
        return Data;
    }
    const char* NullTerminated() const {
        Assert(capacity() >= Len + 1);
        Assert(Data[Len] == _Char(0));
        return Data;
    }

    inline friend TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TBasicStaticString& str) {
        return oss << str.Str();
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
    wdst.Len = FPlatformString::CHAR_to_WCHAR(ECodePage::UTF_8, wdst.Data, wdst.capacity(), src.data(), src.size());
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
    dst.Len = FPlatformString::WCHAR_to_CHAR(ECodePage::ACP, dst.Data, dst.capacity(), wsrc.data(), wsrc.size());
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
