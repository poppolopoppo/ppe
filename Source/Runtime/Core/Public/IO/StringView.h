#pragma once

#include "Core_fwd.h"

#include "IO/String_fwd.h"
#include "Container/Hash.h"
#include "Container/Pair.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryView.h"

#include <cctype>
#include <cwctype>
#include <string.h>
#include <wchar.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CONSTEXPR char ToLower(char ch) NOEXCEPT { return ((ch >= 'A') && (ch <= 'Z')) ? 'a' + (ch - 'A') : ch; }
CONSTEXPR char ToUpper(char ch) NOEXCEPT { return ((ch >= 'a') && (ch <= 'z')) ? 'A' + (ch - 'a') : ch; }
//----------------------------------------------------------------------------
inline wchar_t ToLower(wchar_t wch) NOEXCEPT { return std::towlower(wch); }
inline wchar_t ToUpper(wchar_t wch) NOEXCEPT { return std::towupper(wch); }
//----------------------------------------------------------------------------
template <typename _Char> void InplaceToLower(_Char& ch) NOEXCEPT { ch = ToLower(ch); }
template <typename _Char> void InplaceToUpper(_Char& ch) NOEXCEPT { ch = ToUpper(ch); }
//----------------------------------------------------------------------------
PPE_CORE_API void ToLower(const TMemoryView<char>& dst, const TBasicStringView<char>& src) NOEXCEPT;
PPE_CORE_API void ToLower(const TMemoryView<wchar_t>& dst, const TBasicStringView<wchar_t>& src) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API void ToUpper(const TMemoryView<char>& dst, const TBasicStringView<char>& src) NOEXCEPT;
PPE_CORE_API void ToUpper(const TMemoryView<wchar_t>& dst, const TBasicStringView<wchar_t>& src) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Char> void InplaceToLower(const TMemoryView<_Char>& str) NOEXCEPT { ToLower(str, str); }
template <typename _Char> void InplaceToUpper(const TMemoryView<_Char>& str) NOEXCEPT { ToUpper(str, str); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringView : public TMemoryView< Meta::TAddConst<_Char> > {
public:
    typedef TMemoryView< Meta::TAddConst<_Char> > parent_type;

    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::size_type;
    using typename parent_type::iterator;
    using typename parent_type::reverse_iterator;

    CONSTEXPR TBasicStringView() = default;

    CONSTEXPR TBasicStringView(const TBasicStringView& other) = default;
    CONSTEXPR TBasicStringView& operator =(const TBasicStringView& other) = default;

    CONSTEXPR TBasicStringView(TBasicStringView&& rvalue) = default;
    CONSTEXPR TBasicStringView& operator =(TBasicStringView&& rvalue) = default;

    CONSTEXPR TBasicStringView(const parent_type& other) NOEXCEPT : parent_type(other) {}
    CONSTEXPR TBasicStringView& operator =(const parent_type& other) NOEXCEPT { parent_type::operator =(other); return *this; }

    CONSTEXPR TBasicStringView(const TMemoryView<_Char>& other) NOEXCEPT : parent_type(other) {}
    CONSTEXPR TBasicStringView& operator =(const TMemoryView<_Char>& other) NOEXCEPT { parent_type::operator =(other); return *this; }

    CONSTEXPR TBasicStringView(parent_type&& rvalue) NOEXCEPT : parent_type(std::move(rvalue)) {}
    CONSTEXPR TBasicStringView& operator =(parent_type&& rvalue) NOEXCEPT { parent_type::operator =(std::move(rvalue)); return *this; }

    TBasicStringView(std::initializer_list<value_type> list) NOEXCEPT : parent_type(list) {}
    TBasicStringView(const iterator& first, const iterator& last) NOEXCEPT : parent_type(first, last) {}
    TBasicStringView(const TPair<iterator, iterator>& span) NOEXCEPT : parent_type(span.first, span.second) {}
    CONSTEXPR TBasicStringView(pointer storage, size_type size) NOEXCEPT : parent_type(storage, size) {}

    CONSTEXPR TBasicStringView(TBasicStringLiteral<_Char> literal);
    CONSTEXPR TBasicStringView& operator =(TBasicStringLiteral<_Char> literal);

    template <u32 _Len>
    explicit CONSTEXPR TBasicStringView(const _Char (&arr)[_Len])
    :   parent_type(&arr[0], _Len - 1/* \0 */) {
        STATIC_ASSERT(_Len > 0);
        Assert_NoAssume(arr[_Len - 1] == STRING_LITERAL(_Char, '\0'));
    }

    template <size_t _Dim>
    void ToNullTerminatedCStr(_Char (&dst)[_Dim]) const {
        Assert(_Dim > parent_type::size());
        parent_type::CopyTo(dst);
        dst[parent_type::size()] = _Char(0);
    }

    const parent_type& MakeView() const NOEXCEPT { return *this; }

    auto ToLower() const NOEXCEPT {
        _Char(*transform)(_Char) = &PPE::ToLower;
        return parent_type::Map(transform);
    }

    auto ToUpper() const NOEXCEPT {
        _Char(*transform)(_Char) = &PPE::ToUpper;
        return parent_type::Map(transform);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // override parent member function, which takes a TMemoryView as input: we don't want to have implicit conversion between const _Const (&arr)[N] and TMemoryView<> due to '\0' handling !
    iterator FindSubRange(const TBasicStringView& subrange) const { return parent_type::FindSubRange(subrange); }
    bool EndsWith(_Char suffix) const  { return parent_type::EndsWith(suffix); }
    bool StartsWith(_Char prefix) const { return parent_type::StartsWith(prefix); }
    bool EndsWith(const TBasicStringView& suffix) const { return parent_type::EndsWith(suffix); }
    bool StartsWith(const TBasicStringView& prefix) const { return parent_type::StartsWith(prefix); }
    bool RangeEqual(TBasicStringView other) const { return parent_type::RangeEqual(other); }
    NODISCARD TBasicStringView Concat(const TBasicStringView& other) const { return parent_type::Concat(other); }
    NODISCARD TBasicStringView Concat_AssumeNotEmpty(const TBasicStringView& other) const { return parent_type::Concat_AssumeNotEmpty(other); }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    friend bool operator ==(const TBasicStringView& lhs, const TBasicStringView& rhs) NOEXCEPT { return Equals(lhs, rhs); }
    friend bool operator !=(const TBasicStringView& lhs, const TBasicStringView& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicStringView& lhs, const TBasicStringView& rhs) NOEXCEPT { return (Compare(lhs, rhs) < 0); }
    friend bool operator >=(const TBasicStringView& lhs, const TBasicStringView& rhs) NOEXCEPT { return not operator < (lhs, rhs); }
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FStringView)
PPE_ASSUME_TYPE_AS_POD(FWStringView)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// A string literal always points to static text, is null-terminated, and should not be copied
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringLiteral {
public:
    const _Char* Data{ nullptr };
    size_t Length{ 0 };

    CONSTEXPR TBasicStringLiteral() = default;

    template <u32 _Len>
    CONSTEXPR TBasicStringLiteral(const _Char (&arr)[_Len])
    :   Data(&arr[0])
    ,   Length(_Len - 1/* \0 */) {
        STATIC_ASSERT(_Len > 0);
        Assert_NoAssume(arr[_Len - 1] == STRING_LITERAL(_Char, '\0'));
    }

    NODISCARD CONSTEXPR bool empty() const { return (nullptr == Data); }
    NODISCARD CONSTEXPR size_t size() const { return Length; }

    NODISCARD CONSTEXPR size_t SizeInBytes() const { return (Length * sizeof(_Char)); }

    NODISCARD CONSTEXPR const _Char* c_str() const NOEXCEPT {
        return Data;
    }
    NODISCARD CONSTEXPR TBasicConstChar<_Char> ConstChar() const {
        return { Data };
    }
    NODISCARD CONSTEXPR TBasicStringView<_Char> MakeView() const {
        return { Data, Length };
    }

    NODISCARD CONSTEXPR const _Char* operator *() const { return Data; }

    NODISCARD CONSTEXPR _Char& operator [](size_t index) {
        Assert(index < Length);
        return Data[index];
    }
    NODISCARD CONSTEXPR _Char operator [](size_t index) const {
        Assert(index < Length);
        return Data[index];
    }

    // NODISCARD CONSTEXPR operator const _Char* () const { return c_str(); } // *AMBIGUOUS*, use operator *() instead
    NODISCARD CONSTEXPR operator TBasicConstChar<_Char> () const { return ConstChar(); }
    // NODISCARD CONSTEXPR operator TBasicStringView<_Char> () const { return MakeView(); } // *AMBIGUOUS* with TBasicStringView<>(TBasicStringLiteral<>)

    friend bool operator ==(const TBasicStringLiteral& lhs, const TBasicStringLiteral& rhs) NOEXCEPT { return Equals(lhs.MakeView(), rhs.MakeView()); }
    friend bool operator !=(const TBasicStringLiteral& lhs, const TBasicStringLiteral& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicStringLiteral& lhs, const TBasicStringLiteral& rhs) NOEXCEPT { return (Compare(lhs.MakeView(), rhs.MakeView()) < 0); }
    friend bool operator >=(const TBasicStringLiteral& lhs, const TBasicStringLiteral& rhs) NOEXCEPT { return not operator < (lhs, rhs); }
};
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR FStringLiteral operator ""_literal(const char* str, size_t len) NOEXCEPT {
    FStringLiteral result;
    result.Data = str;
    result.Length = len;
    return result;
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR FWStringLiteral operator ""_literal(const wchar_t* wstr, size_t len) NOEXCEPT {
    FWStringLiteral result;
    result.Data = wstr;
    result.Length = len;
    return result;
}
//----------------------------------------------------------------------------
template <typename _CharA, typename _CharB>
TBasicTextWriter<_CharA>& operator <<(TBasicTextWriter<_CharA>& oss, const TBasicStringLiteral<_CharB>& literal) {
    return oss << literal.MakeView();
}
//----------------------------------------------------------------------------
template <typename _Char>
CONSTEXPR TBasicStringView<_Char>::TBasicStringView(TBasicStringLiteral<_Char> literal)
:   TBasicStringView(literal.MakeView())
{}
//----------------------------------------------------------------------------
template <typename _Char>
CONSTEXPR TBasicStringView<_Char>& TBasicStringView<_Char>::operator =(TBasicStringLiteral<_Char> literal) {
    return operator =(literal.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR FStringView operator ""_view(const char* str, size_t len) NOEXCEPT {
    return { str, len };
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR FWStringView operator ""_view(const wchar_t* wstr, size_t len) NOEXCEPT {
    return { wstr, len };
}
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD TBasicStringView<_Char> MakeStringView(const TBasicString<_Char>& str) NOEXCEPT {
    return TBasicStringView<_Char>(str.data(), str.size());
}
//----------------------------------------------------------------------------
template <size_t _Dim>
NODISCARD CONSTEXPR FStringView MakeStringView(const char(&cstr)[_Dim]) NOEXCEPT {
    return FStringView(MakeStringLiteral(cstr));
}
//----------------------------------------------------------------------------
template <size_t _Dim>
NODISCARD CONSTEXPR FWStringView MakeStringView(const wchar_t(&cstr)[_Dim]) NOEXCEPT {
    return FWStringView(MakeStringLiteral(cstr));
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR FStringView MakeStringView(const TMemoryView<const char>& view) NOEXCEPT {
    return FStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR FWStringView MakeStringView(const TMemoryView<const wchar_t>& view) NOEXCEPT {
    return FWStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR FStringView MakeStringView(const TMemoryView<char>& view) NOEXCEPT {
    return FStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR FWStringView MakeStringView(const TMemoryView<wchar_t>& view) NOEXCEPT {
    return FWStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR const FStringView& MakeStringView(const FStringView& view) NOEXCEPT {
    return view;
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR const FWStringView& MakeStringView(const FWStringView& view) NOEXCEPT {
    return view;
}
//----------------------------------------------------------------------------
template <typename _It>
NODISCARD typename std::enable_if<
    Meta::is_iterator_of<_It, char>::value,
    FStringView
>::type CONSTF MakeStringView(_It first, _It last) NOEXCEPT {
    typedef std::iterator_traits<_It> traits_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return FStringView(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
template <typename _It>
NODISCARD typename std::enable_if<
    Meta::is_iterator_of<_It, wchar_t>::value,
    FWStringView
>::type CONSTF MakeStringView(_It first, _It last) NOEXCEPT {
    typedef std::iterator_traits<_It> traits_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return FWStringView(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD inline size_t CONSTF Length(const char* string) { return ::strlen(string); }
NODISCARD inline size_t CONSTF Length(const wchar_t* string) { return ::wcslen(string); }
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD FORCE_INLINE TBasicStringView<_Char> CONSTF MakeCStringView(const _Char* cstr) NOEXCEPT {
    return TBasicStringView<_Char>(cstr, cstr ? Length(cstr) : 0);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD inline const char* CONSTF StrChr(const char* cstr, char ch) NOEXCEPT { return ::strchr(cstr, ch); }
NODISCARD inline const char* CONSTF StrRChr(const char* cstr, char ch) NOEXCEPT { return ::strrchr(cstr, ch); }
//----------------------------------------------------------------------------
NODISCARD inline const wchar_t* CONSTF StrChr(const wchar_t* wcstr, wchar_t wch) NOEXCEPT { return ::wcschr(wcstr, wch); }
NODISCARD inline const wchar_t* CONSTF StrRChr(const wchar_t* wcstr, wchar_t wch) NOEXCEPT { return ::wcsrchr(wcstr, wch); }
//----------------------------------------------------------------------------
NODISCARD inline const char* CONSTF StrStr(const char* cstr, const char* firstOccurence) NOEXCEPT { return ::strstr(cstr, firstOccurence); }
NODISCARD inline const wchar_t* CONSTF StrStr(const wchar_t* wcstr, const wchar_t* firstOccurence) NOEXCEPT { return ::wcsstr(wcstr, firstOccurence); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView::iterator CONSTF StrChr(const FStringView& str, char ch) NOEXCEPT;
NODISCARD PPE_CORE_API FStringView::reverse_iterator CONSTF StrRChr(const FStringView& str, char ch) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FWStringView::iterator CONSTF StrChr(const FWStringView& wstr, wchar_t wch) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView::reverse_iterator CONSTF StrRChr(const FWStringView& wstr, wchar_t wch) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView::iterator CONSTF StrStr(const FStringView& str, const FStringView& firstOccurence) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView::iterator CONSTF StrStr(const FWStringView& wstr, const FWStringView& firstOccurence) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView::iterator CONSTF StrStrI(const FStringView& str, const FStringView& firstOccurence) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView::iterator CONSTF StrStrI(const FWStringView& wstr, const FWStringView& firstOccurence) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD inline bool CONSTF HasSubString(const FStringView& str, const FStringView& substr) NOEXCEPT { return str.end() != StrStr(str, substr); }
NODISCARD inline bool CONSTF HasSubString(const FWStringView& str, const FWStringView& substr) NOEXCEPT { return str.end() != StrStr(str, substr); }
//----------------------------------------------------------------------------
NODISCARD inline bool CONSTF HasSubStringI(const FStringView& str, const FStringView& substr) NOEXCEPT { return str.end() != StrStrI(str, substr); }
NODISCARD inline bool CONSTF HasSubStringI(const FWStringView& str, const FWStringView& substr) NOEXCEPT { return str.end() != StrStrI(str, substr); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Used LLVM to reverse the optimal ASM generated to C: https://godbolt.org/z/hanWdq
//----------------------------------------------------------------------------
#if 1 //optimized ASM
NODISCARD CONSTEXPR bool CONSTF IsAlpha(char ch) NOEXCEPT { return (static_cast<uint8_t>('z' - 'a' + 1) > static_cast<uint8_t>((ch & ('A' - 'a' - 1)) - 'A')); }
#else
NODISCARD CONSTEXPR bool CONSTF IsAlpha(char ch) NOEXCEPT { return ( ( ch >= 'a' ) & ( ch <= 'z' ) ) | ( ( ch >= 'A' ) & ( ch <= 'Z' ) ); }
#endif
NODISCARD inline bool CONSTF IsAlpha(wchar_t wch) NOEXCEPT { return 0 != std::iswalpha(wch); }
//----------------------------------------------------------------------------
#if 1 //optimized ASM
NODISCARD CONSTEXPR bool CONSTF IsDigit(char ch) NOEXCEPT { return (static_cast<uint8_t>('9' - '0' + 1) > static_cast<uint8_t>(ch - '0')); }
#else
CONSTEXPR bool CONSTF IsDigit(char ch) NOEXCEPT { return ( ( ch >= '0' ) & ( ch <= '9' ) ); }
#endif
NODISCARD inline bool CONSTF IsDigit(wchar_t wch) NOEXCEPT { return 0 != std::iswdigit(wch); }
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR bool CONSTF IsEndLine(char ch) NOEXCEPT { return ( ( ch == '\r' ) || ( ch == '\n' ) ); }
NODISCARD CONSTEXPR bool CONSTF IsEndLine(wchar_t wch) NOEXCEPT { return ( ( wch == L'\r' ) || ( wch == L'\n' ) ); }
//----------------------------------------------------------------------------
#if 1 //optimized ASM
NODISCARD CONSTEXPR bool CONSTF IsXDigit(char ch) NOEXCEPT { return (
    (static_cast<uint8_t>('9' - '0' + 1) > static_cast<uint8_t>(ch - '0')) ||
    (static_cast<uint8_t>('f' - 'a' + 1) > static_cast<uint8_t>((ch & ('A' - 'a'  - 1)) - 'A')) ); }
#else
NODISCARD CONSTEXPR bool CONSTF IsXDigit(char ch) NOEXCEPT { return IsDigit(ch) | ( ( ch >= 'a' ) & ( ch <= 'f' ) ) | ( ( ch >= 'A' ) & ( ch <= 'F' ) ); }
#endif
NODISCARD inline bool CONSTF IsXDigit(wchar_t wch) NOEXCEPT { return 0 != std::iswxdigit(wch); }
//----------------------------------------------------------------------------
#if 1 //optimized ASM
NODISCARD CONSTEXPR bool CONSTF IsAlnum(char ch) NOEXCEPT { return (
    (static_cast<uint8_t>('z' - 'a' + 1) > static_cast<uint8_t>((ch & ('A' - 'a' - 1)) - 'A')) ||
    (static_cast<uint8_t>('9' - '0' + 1) > static_cast<uint8_t>(ch - '0'))  ); }
#else
NODISCARD CONSTEXPR bool CONSTF IsAlnum(char ch) NOEXCEPT { return (IsAlpha(ch) | IsDigit(ch)); }
#endif
NODISCARD inline bool CONSTF IsAlnum(wchar_t wch) NOEXCEPT { return 0 != std::iswalnum(wch); }
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR bool CONSTF IsIdentifier(char ch) NOEXCEPT { return (IsAlnum(ch) || ( ch == '_' ) || ( ch == '.' )); }
NODISCARD inline bool CONSTF IsIdentifier(wchar_t wch) NOEXCEPT { return (IsAlnum(wch) || ((wch == L'_') || (wch == L'.'))); }
//----------------------------------------------------------------------------
#if 1 //optimized ASM
NODISCARD CONSTEXPR bool CONSTF IsOctal(char ch) NOEXCEPT { return (static_cast<uint8_t>('7' - '0' + 1) > static_cast<uint8_t>(ch - '0')); }
#else
NODISCARD CONSTEXPR bool CONSTF IsOctal(char ch) NOEXCEPT { return ( ( ch >= '0' ) & ( ch <= '7' ) ); }
#endif
NODISCARD inline bool CONSTF IsOctal(wchar_t wch) NOEXCEPT { return (IsDigit(wch) && (wch != '8') && (wch != '9')); }
//----------------------------------------------------------------------------
NODISCARD inline bool CONSTF IsPrint(char ch) NOEXCEPT { return 0 != std::isprint((int)((unsigned char)ch)); }
NODISCARD inline bool CONSTF IsPrint(wchar_t wch) NOEXCEPT { return 0 != std::iswprint(wch); }
//----------------------------------------------------------------------------
NODISCARD inline bool CONSTF IsPunct(char ch) NOEXCEPT { return 0 != std::ispunct((int)((unsigned char)ch)); }
NODISCARD inline bool CONSTF IsPunct(wchar_t wch) NOEXCEPT { return 0 != std::iswpunct(wch); }
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR bool CONSTF IsSpace(char ch) NOEXCEPT { return ((ch == ' ') || (ch == '\f') || (ch == '\n') || (ch == '\r') || (ch == '\t') || (ch == '\v')); }
NODISCARD inline bool CONSTF IsSpace(wchar_t wch) NOEXCEPT { return 0 != std::iswspace(wch); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF IsAlnum(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF IsAlnum(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF IsAlpha(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF IsAlpha(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF IsDigit(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF IsDigit(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF IsXDigit(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF IsXDigit(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF IsIdentifier(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF IsIdentifier(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF IsOctal(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF IsOctal(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF IsPrint(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF IsPrint(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF IsSpace(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF IsSpace(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF HasAlnum(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF HasAlnum(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF HasAlpha(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF HasAlpha(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF HasDigit(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF HasDigit(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF HasXDigit(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF HasXDigit(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF HasPrint(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF HasPrint(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF HasSpace(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF HasSpace(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatAlnums(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatAlnums(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatAlphas(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatAlphas(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatDigits(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatDigits(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatXDigits(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatXDigits(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatIdentifier(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatIdentifier(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatOctals(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatOctals(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatPrints(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatPrints(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatSpaces(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatSpaces(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatWhile(FStringView& str, char ch) NOEXCEPT;
PPE_CORE_API FWStringView EatWhile(FWStringView& wstr, wchar_t wch) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatWhile(FStringView& str, const FStringView& multiple) NOEXCEPT;
PPE_CORE_API FWStringView EatWhile(FWStringView& wstr, const FWStringView& wmultiple) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatWhile(FStringView& str, bool (*is_a)(char)) NOEXCEPT;
PPE_CORE_API FWStringView EatWhile(FWStringView& wstr, bool (*is_a)(wchar_t)) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatUntil(FStringView& str, char ch) NOEXCEPT;
PPE_CORE_API FWStringView EatUntil(FWStringView& wstr, wchar_t wch) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatUntil(FStringView& str, const FStringView& multiple) NOEXCEPT;
PPE_CORE_API FWStringView EatUntil(FWStringView& wstr, const FWStringView& wmultiple) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatUntil(FStringView& str, bool (*is_not_a)(char)) NOEXCEPT;
PPE_CORE_API FWStringView EatUntil(FWStringView& wstr, bool (*is_not_a)(wchar_t)) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatLine(FStringView& str) NOEXCEPT;
PPE_CORE_API FWStringView EatLine(FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool EatExpect(FStringView& str, const FStringView& expected) NOEXCEPT;
NODISCARD PPE_CORE_API bool EatExpect(FWStringView& wstr, const FWStringView& expected) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool EatExpectI(FStringView& str, const FStringView& expected) NOEXCEPT;
NODISCARD PPE_CORE_API bool EatExpectI(FWStringView& wstr, const FWStringView& expected) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView TrimStart(const FStringView& str, char ch) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView TrimStart(const FWStringView& wstr, wchar_t wch) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView TrimStart(const FStringView& str, const FStringView& chars) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView TrimStart(const FWStringView& wstr, const FWStringView& wchars) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView TrimEnd(const FStringView& str, char ch) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView TrimEnd(const FWStringView& wstr, wchar_t wch) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView TrimEnd(const FStringView& str, const FStringView& chars) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView TrimEnd(const FWStringView& wstr, const FWStringView& wchars) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView Trim(const FStringView& str, char ch) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView Trim(const FWStringView& wstr, wchar_t wch) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView Trim(const FStringView& str, const FStringView& chars) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView Trim(const FWStringView& wstr, const FWStringView& wchars) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView Chomp(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView Chomp(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API FStringView Strip(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API FWStringView Strip(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API int CONSTF Compare(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
NODISCARD PPE_CORE_API int CONSTF Compare(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API int CONSTF CompareI(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
NODISCARD PPE_CORE_API int CONSTF CompareI(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF EqualsN(const char* lhs, const char* rhs, size_t len) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF EqualsNI(const char* lhs, const char* rhs, size_t len) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF EqualsN(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF EqualsNI(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF Equals(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF Equals(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD inline bool CONSTF Equals(const FStringView& lhs, const FStringLiteral& rhs) NOEXCEPT { return Equals(lhs, rhs.MakeView()); }
NODISCARD inline bool CONSTF Equals(const FStringLiteral& lhs, const FStringView& rhs) NOEXCEPT { return Equals(lhs.MakeView(), rhs); }
NODISCARD inline bool CONSTF Equals(const FWStringView& lhs, const FWStringLiteral& rhs) NOEXCEPT { return Equals(lhs, rhs.MakeView()); }
NODISCARD inline bool CONSTF Equals(const FWStringLiteral& lhs, const FWStringView& rhs) NOEXCEPT { return Equals(lhs.MakeView(), rhs); }
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF EqualsI(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF EqualsI(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD inline bool CONSTF EqualsI(const FStringView& lhs, const FStringLiteral& rhs) NOEXCEPT { return EqualsI(lhs, rhs.MakeView()); }
NODISCARD inline bool CONSTF EqualsI(const FStringLiteral& lhs, const FStringView& rhs) NOEXCEPT { return EqualsI(lhs.MakeView(), rhs); }
NODISCARD inline bool CONSTF EqualsI(const FWStringView& lhs, const FWStringLiteral& rhs) NOEXCEPT { return EqualsI(lhs, rhs.MakeView()); }
NODISCARD inline bool CONSTF EqualsI(const FWStringLiteral& lhs, const FWStringView& rhs) NOEXCEPT { return EqualsI(lhs.MakeView(), rhs); }
//----------------------------------------------------------------------------
NODISCARD inline bool CONSTF EqualsI(char lhs, char rhs) NOEXCEPT { return (ToUpper(lhs) == ToUpper(rhs)); }
NODISCARD inline bool CONSTF EqualsI(wchar_t lhs, wchar_t rhs) NOEXCEPT { return (ToUpper(lhs) == ToUpper(rhs)); }
//----------------------------------------------------------------------------
NODISCARD inline bool CONSTF Equals(char lhs, char rhs, ECase cmp) NOEXCEPT { return (ECase::Insensitive == cmp ? ToUpper(lhs) == ToUpper(rhs) : lhs == rhs); }
NODISCARD inline bool CONSTF Equals(wchar_t lhs, wchar_t rhs, ECase cmp) NOEXCEPT { return (ECase::Insensitive == cmp ? ToUpper(lhs) == ToUpper(rhs) : lhs == rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF StartsWith(const FStringView& str, const FStringView& prefix) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF StartsWith(const FWStringView& wstr, const FWStringView& wprefix) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF StartsWithI(const FStringView& str, const FStringView& prefix) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF StartsWithI(const FWStringView& wstr, const FWStringView& wprefix) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF EndsWith(const FStringView& str, const FStringView& suffix) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF EndsWith(const FWStringView& wstr, const FWStringView& wsuffix) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF EndsWithI(const FStringView& str, const FStringView& suffix) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF EndsWithI(const FWStringView& wstr, const FWStringView& wsuffix) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF StartsWithI(const FStringView& str, const FStringView& prefix) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF StartsWithI(const FWStringView& wstr, const FWStringView& wprefix) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF EndsWith(const FStringView& str, const FStringView& suffix) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF EndsWith(const FWStringView& wstr, const FWStringView& wsuffix) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool CONSTF EndsWithI(const FStringView& str, const FStringView& suffix) NOEXCEPT;
NODISCARD PPE_CORE_API bool CONSTF EndsWithI(const FWStringView& wstr, const FWStringView& wsuffix) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool Split(FStringView& str, char separator, FStringView& slice) NOEXCEPT;
NODISCARD PPE_CORE_API bool Split(FWStringView& wstr, wchar_t separator, FWStringView& slice) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool Split(FStringView& str, const FStringView& separators, FStringView& slice) NOEXCEPT;
NODISCARD PPE_CORE_API bool Split(FWStringView& wstr, const FWStringView& separators, FWStringView& slice) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool SplitR(FStringView& str, char separator, FStringView& slice) NOEXCEPT;
NODISCARD PPE_CORE_API bool SplitR(FWStringView& wstr, wchar_t separator, FWStringView& slice) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool SplitR(FStringView& str, const FStringView& separators, FStringView& slice) NOEXCEPT;
NODISCARD PPE_CORE_API bool SplitR(FWStringView& wstr, const FWStringView& separators, FWStringView& slice) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool SplitNth(FStringView& str, char separator, FStringView& slice, size_t nth) NOEXCEPT;
NODISCARD PPE_CORE_API bool SplitNth(FWStringView& wstr, wchar_t separator, FWStringView& slice, size_t nth) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool SplitNth(FStringView& str, const FStringView& separators, FStringView& slice, size_t nth) NOEXCEPT;
NODISCARD PPE_CORE_API bool SplitNth(FWStringView& wstr, const FWStringView& separators, FWStringView& slice, size_t nth) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool SplitNthR(FStringView& str, char separator, FStringView& slice, size_t nth) NOEXCEPT;
NODISCARD PPE_CORE_API bool SplitNthR(FWStringView& wstr, wchar_t separator, FWStringView& slice, size_t nth) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool SplitNthR(FStringView& str, const FStringView& separators, FStringView& slice, size_t nth) NOEXCEPT;
NODISCARD PPE_CORE_API bool SplitNthR(FWStringView& wstr, const FWStringView& separators, FWStringView& slice, size_t nth) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD TBasicStringView<_Char> Slice(const TBasicStringView<_Char>& str, size_t offset, size_t count)  NOEXCEPT {
    return str.SubRange(offset, count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool Atoi(i32* dst, const FStringView& str, u32 base) NOEXCEPT;
NODISCARD PPE_CORE_API bool Atoi(u32* dst, const FStringView& str, u32 base) NOEXCEPT;
NODISCARD PPE_CORE_API bool Atoi(i64* dst, const FStringView& str, u32 base) NOEXCEPT;
NODISCARD PPE_CORE_API bool Atoi(u64* dst, const FStringView& str, u32 base) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool Atoi(i32* dst, const FWStringView& wstr, u32 base) NOEXCEPT;
NODISCARD PPE_CORE_API bool Atoi(u32* dst, const FWStringView& wstr, u32 base) NOEXCEPT;
NODISCARD PPE_CORE_API bool Atoi(i64* dst, const FWStringView& wstr, u32 base) NOEXCEPT;
NODISCARD PPE_CORE_API bool Atoi(u64* dst, const FWStringView& wstr, u32 base) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool Atof(float* dst, const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool Atod(double* dst, const FStringView& str) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool Atof(float* dst, const FWStringView& wstr) NOEXCEPT;
NODISCARD PPE_CORE_API bool Atod(double* dst, const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool WildMatch(const FStringView& pattern, const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool WildMatch(const FWStringView& pattern, const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool WildMatchI(const FStringView& pattern, const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API bool WildMatchI(const FWStringView& pattern, const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t LevenshteinDistance(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
NODISCARD PPE_CORE_API size_t LevenshteinDistance(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t LevenshteinDistanceI(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
NODISCARD PPE_CORE_API size_t LevenshteinDistanceI(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API size_t Copy(const TMemoryView<char>& dst, const FStringView& src);
NODISCARD PPE_CORE_API size_t Copy(const TMemoryView<wchar_t>& dst, const FWStringView& src);
//----------------------------------------------------------------------------
PPE_CORE_API const char* NullTerminated(const TMemoryView<char>& dst, const FStringView& src);
PPE_CORE_API const wchar_t* NullTerminated(const TMemoryView<wchar_t>& dst, const FWStringView& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API hash_t hash_string(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API hash_t hash_string(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API hash_t hash_stringI(const FStringView& str) NOEXCEPT;
NODISCARD PPE_CORE_API hash_t hash_stringI(const FWStringView& wstr) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EEscape {
    Octal = 0,
    Hexadecimal,
    Unicode,
};
//----------------------------------------------------------------------------
PPE_CORE_API void Escape(FTextWriter& oss, const FStringView& str, EEscape escape);
PPE_CORE_API void Escape(FTextWriter& oss, const FWStringView& str, EEscape escape);
PPE_CORE_API void Escape(FWTextWriter& oss, const FStringView& str, EEscape escape);
PPE_CORE_API void Escape(FWTextWriter& oss, const FWStringView& str, EEscape escape);
//----------------------------------------------------------------------------
PPE_CORE_API void Unescape(FTextWriter& oss, const FStringView& str);
PPE_CORE_API void Unescape(FWTextWriter& oss, const FWStringView& str);
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FWStringView& wslice);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FStringView& slice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TCharEqualTo {
    NODISCARD CONSTEXPR bool operator ()(const _Char& lhs, const _Char& rhs) const NOEXCEPT { return lhs == rhs; }
};
template <typename _Char>
struct TCharEqualTo<_Char, ECase::Insensitive> {
    NODISCARD CONSTEXPR bool operator ()(const _Char& lhs, const _Char& rhs) const NOEXCEPT { return ToLower(lhs) == ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TCharLess {
    NODISCARD CONSTEXPR bool operator ()(const _Char& lhs, const _Char& rhs) const NOEXCEPT { return lhs < rhs; }
};
template <typename _Char>
struct TCharLess<_Char, ECase::Insensitive> {
    NODISCARD CONSTEXPR bool operator ()(const _Char& lhs, const _Char& rhs) const NOEXCEPT { return ToLower(lhs) < ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TCharCase {
    NODISCARD CONSTEXPR _Char operator ()(const _Char& ch) const NOEXCEPT { return ch; }
};
template <typename _Char>
struct TCharCase<_Char, ECase::Insensitive> {
    NODISCARD CONSTEXPR _Char operator ()(const _Char& ch) const NOEXCEPT { return ToLower(ch); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD CONSTEXPR auto MakeCaseInsensitive(const TBasicStringView<_Char>& str) NOEXCEPT {
    return MakeOutputIterable(str.data(), str.data() + str.size(), TCharCase<_Char, ECase::Insensitive>{});
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
NODISCARD CONSTEXPR auto MakeCaseInsensitive(const _Char (&staticChars)[_Dim]) NOEXCEPT {
    return MakeCaseInsensitive(MakeStringView(staticChars));
}
//----------------------------------------------------------------------------
using FCaseInsensitive = decltype(MakeCaseInsensitive(std::declval<FStringView>()));
using FWCaseInsensitive = decltype(MakeCaseInsensitive(std::declval<FWStringView>()));
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewEqualTo {
    NODISCARD bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT {
        return Equals(lhs, rhs);
    }
};
template <typename _Char>
struct TStringViewEqualTo<_Char, ECase::Insensitive> {
    NODISCARD bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT {
        return EqualsI(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewLess {
    NODISCARD bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT {
        return (Compare(lhs, rhs) < 0);
    }
};
template <typename _Char>
struct TStringViewLess<_Char, ECase::Insensitive> {
    NODISCARD bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT {
        return (CompareI(lhs, rhs) < 0);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewHasher {
    NODISCARD hash_t operator ()(const TBasicStringView<_Char>& str) const NOEXCEPT {
        return hash_string(str);
    }
};
template <typename _Char>
struct TStringViewHasher<_Char, ECase::Insensitive> {
    NODISCARD hash_t operator ()(const TBasicStringView<_Char>& str) const NOEXCEPT {
        return hash_stringI(str);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
