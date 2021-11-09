#pragma once

#include "Core_fwd.h"

#include "IO/String_fwd.h"
#include "Container/Hash.h"
#include "Container/Pair.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryView.h"

#include <cctype>
#include <cwctype>
#include <locale>
#include <string.h>
#include <wchar.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ECase : bool {
    Sensitive   = true,
    Insensitive = false,
};
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

    template <size_t _Dim>
    CONSTEXPR TBasicStringView(const _Char(&staticChars)[_Dim]) NOEXCEPT
    :   parent_type(staticChars, _Dim - 1/* assume null terminated string */) {
        static_assert(_Dim, "invalid string");
    }

    template <size_t _Dim>
    CONSTEXPR TBasicStringView& operator =(const _Char(&staticChars)[_Dim]) NOEXCEPT {
        static_assert(_Dim, "invalid string");
        return (*this = TBasicStringView(staticChars, _Dim - 1));
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
CONSTEXPR FStringView operator "" _view(const char* str, size_t len) {
    return { str, len };
}
//----------------------------------------------------------------------------
CONSTEXPR FWStringView operator "" _view(const wchar_t* wstr, size_t len) {
    return { wstr, len };
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> MakeStringView(const TBasicString<_Char>& str) {
    return TBasicStringView<_Char>(str.data(), str.size());
}
//----------------------------------------------------------------------------
template <size_t _Dim>
CONSTEXPR FStringView MakeStringView(const char(&cstr)[_Dim]) {
    return FStringView(cstr);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
CONSTEXPR FWStringView MakeStringView(const wchar_t(&cstr)[_Dim]) {
    return FWStringView(cstr);
}
//----------------------------------------------------------------------------
CONSTEXPR FStringView MakeStringView(const TMemoryView<const char>& view) {
    return FStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
CONSTEXPR FWStringView MakeStringView(const TMemoryView<const wchar_t>& view) {
    return FWStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
CONSTEXPR FStringView MakeStringView(const TMemoryView<char>& view) {
    return FStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
CONSTEXPR FWStringView MakeStringView(const TMemoryView<wchar_t>& view) {
    return FWStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
CONSTEXPR const FStringView& MakeStringView(const FStringView& view) {
    return view;
}
//----------------------------------------------------------------------------
CONSTEXPR const FWStringView& MakeStringView(const FWStringView& view) {
    return view;
}
//----------------------------------------------------------------------------
template <typename _It>
typename std::enable_if<
    Meta::is_iterator_of<_It, char>::value,
    FStringView
>::type MakeStringView(_It first, _It last) {
    typedef std::iterator_traits<_It> traits_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return FStringView(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
template <typename _It>
typename std::enable_if<
    Meta::is_iterator_of<_It, wchar_t>::value,
    FWStringView
>::type MakeStringView(_It first, _It last) {
    typedef std::iterator_traits<_It> traits_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return FWStringView(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline size_t Length(const char* string) { return ::strlen(string); }
inline size_t Length(const wchar_t* string) { return ::wcslen(string); }
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE TBasicStringView<_Char> MakeCStringView(const _Char* cstr) {
    return (nullptr != cstr)
        ? TBasicStringView<_Char>(cstr, Length(cstr))
        : TBasicStringView<_Char>();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline const char *StrChr(const char* cstr, char ch) { return ::strchr(cstr, ch); }
inline const char *StrRChr(const char* cstr, char ch) { return ::strrchr(cstr, ch); }
//----------------------------------------------------------------------------
inline const wchar_t *StrChr(const wchar_t* wcstr, wchar_t wch) { return ::wcschr(wcstr, wch); }
inline const wchar_t *StrRChr(const wchar_t* wcstr, wchar_t wch) { return ::wcsrchr(wcstr, wch); }
//----------------------------------------------------------------------------
inline const char *StrStr(const char* cstr, const char* firstOccurence) { return ::strstr(cstr, firstOccurence); }
inline const wchar_t *StrStr(const wchar_t* wcstr, const wchar_t* firstOccurence) { return ::wcsstr(wcstr, firstOccurence); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FStringView::iterator StrChr(const FStringView& str, char ch);
PPE_CORE_API FStringView::reverse_iterator StrRChr(const FStringView& str, char ch);
//----------------------------------------------------------------------------
PPE_CORE_API FWStringView::iterator StrChr(const FWStringView& wstr, wchar_t wch);
PPE_CORE_API FWStringView::reverse_iterator StrRChr(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView::iterator StrStr(const FStringView& str, const FStringView& firstOccurence);
PPE_CORE_API FWStringView::iterator StrStr(const FWStringView& wstr, const FWStringView& firstOccurence);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView::iterator StrStrI(const FStringView& str, const FStringView& firstOccurence);
PPE_CORE_API FWStringView::iterator StrStrI(const FWStringView& wstr, const FWStringView& firstOccurence);
//----------------------------------------------------------------------------
inline bool HasSubString(const FStringView& str, const FStringView& substr) { return str.end() != StrStr(str, substr); }
inline bool HasSubString(const FWStringView& str, const FWStringView& substr) { return str.end() != StrStr(str, substr); }
//----------------------------------------------------------------------------
inline bool HasSubStringI(const FStringView& str, const FStringView& substr) { return str.end() != StrStrI(str, substr); }
inline bool HasSubStringI(const FWStringView& str, const FWStringView& substr) { return str.end() != StrStrI(str, substr); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Used LLVM to reverse the optimal ASM generated to C: https://godbolt.org/z/hanWdq
//----------------------------------------------------------------------------
#if 1 //optimized ASM
CONSTEXPR bool IsAlpha(char ch) { return (static_cast<uint8_t>('z' - 'a' + 1) > static_cast<uint8_t>((ch & ('A' - 'a' - 1)) - 'A')); }
#else
CONSTEXPR bool IsAlpha(char ch) { return ( ( ch >= 'a' ) & ( ch <= 'z' ) ) | ( ( ch >= 'A' ) & ( ch <= 'Z' ) ); }
#endif
inline bool IsAlpha(wchar_t wch) { return 0 != std::iswalpha(wch); }
//----------------------------------------------------------------------------
#if 1 //optimized ASM
CONSTEXPR bool IsDigit(char ch) { return (static_cast<uint8_t>('9' - '0' + 1) > static_cast<uint8_t>(ch - '0')); }
#else
CONSTEXPR bool IsDigit(char ch) { return ( ( ch >= '0' ) & ( ch <= '9' ) ); }
#endif
inline bool IsDigit(wchar_t wch) { return 0 != std::iswdigit(wch); }
//----------------------------------------------------------------------------
CONSTEXPR bool IsEndLine(char ch) { return ( ( ch == '\r' ) | ( ch == '\n' ) ); }
CONSTEXPR bool IsEndLine(wchar_t wch) { return ( ( wch == L'\r' ) | ( wch == L'\n' ) ); }
//----------------------------------------------------------------------------
#if 1 //optimized ASM
CONSTEXPR bool IsXDigit(char ch) { return (
    (static_cast<uint8_t>('9' - '0' + 1) > static_cast<uint8_t>(ch - '0')) |
    (static_cast<uint8_t>('f' - 'a' + 1) > static_cast<uint8_t>((ch & ('A' - 'a'  - 1)) - 'A')) ); }
#else
CONSTEXPR bool IsXDigit(char ch) { return IsDigit(ch) | ( ( ch >= 'a' ) & ( ch <= 'f' ) ) | ( ( ch >= 'A' ) & ( ch <= 'F' ) ); }
#endif
inline bool IsXDigit(wchar_t wch) { return 0 != std::iswxdigit(wch); }
//----------------------------------------------------------------------------
#if 1 //optimized ASM
CONSTEXPR bool IsAlnum(char ch) { return (
    (static_cast<uint8_t>('z' - 'a' + 1) > static_cast<uint8_t>((ch & ('A' - 'a' - 1)) - 'A')) |
    (static_cast<uint8_t>('9' - '0' + 1) > static_cast<uint8_t>(ch - '0'))  ); }
#else
CONSTEXPR bool IsAlnum(char ch) { return (IsAlpha(ch) | IsDigit(ch)); }
#endif
inline bool IsAlnum(wchar_t wch) { return 0 != std::iswalnum(wch); }
//----------------------------------------------------------------------------
CONSTEXPR bool IsIdentifier(char ch) { return (IsAlnum(ch) | ( ch == '_' ) | ( ch == '.' )); }
inline bool IsIdentifier(wchar_t wch) { return (IsAlnum(wch) || ((wch == L'_') | (wch == L'.'))); }
//----------------------------------------------------------------------------
#if 1 //optimized ASM
CONSTEXPR bool IsOctal(char ch) { return (static_cast<uint8_t>('7' - '0' + 1) > static_cast<uint8_t>(ch - '0')); }
#else
CONSTEXPR bool IsOctal(char ch) { return ( ( ch >= '0' ) & ( ch <= '7' ) ); }
#endif
inline bool IsOctal(wchar_t wch) { return (IsDigit(wch) & (wch != '8') & (wch != '9')); }
//----------------------------------------------------------------------------
inline bool IsPrint(char ch) { return 0 != std::isprint((int)((unsigned char)ch)); }
inline bool IsPrint(wchar_t wch) { return 0 != std::iswprint(wch); }
//----------------------------------------------------------------------------
inline bool IsPunct(char ch) { return 0 != std::ispunct((int)((unsigned char)ch)); }
inline bool IsPunct(wchar_t wch) { return 0 != std::iswpunct(wch); }
//----------------------------------------------------------------------------
CONSTEXPR bool IsSpace(char ch) { return ((ch == ' ') | (ch == '\f') | (ch == '\n') | (ch == '\r') | (ch == '\t') | (ch == '\v')); }
inline bool IsSpace(wchar_t wch) { return 0 != std::iswspace(wch); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API bool IsAlnum(const FStringView& str);
PPE_CORE_API bool IsAlnum(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool IsAlpha(const FStringView& str);
PPE_CORE_API bool IsAlpha(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool IsDigit(const FStringView& str);
PPE_CORE_API bool IsDigit(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool IsXDigit(const FStringView& str);
PPE_CORE_API bool IsXDigit(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool IsIdentifier(const FStringView& str);
PPE_CORE_API bool IsIdentifier(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool IsOctal(const FStringView& str);
PPE_CORE_API bool IsOctal(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool IsPrint(const FStringView& str);
PPE_CORE_API bool IsPrint(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool IsSpace(const FStringView& str);
PPE_CORE_API bool IsSpace(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API bool HasAlnum(const FStringView& str);
PPE_CORE_API bool HasAlnum(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool HasAlpha(const FStringView& str);
PPE_CORE_API bool HasAlpha(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool HasDigit(const FStringView& str);
PPE_CORE_API bool HasDigit(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool HasXDigit(const FStringView& str);
PPE_CORE_API bool HasXDigit(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool HasPrint(const FStringView& str);
PPE_CORE_API bool HasPrint(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool HasSpace(const FStringView& str);
PPE_CORE_API bool HasSpace(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatAlnums(FStringView& str);
PPE_CORE_API FWStringView EatAlnums(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatAlphas(FStringView& str);
PPE_CORE_API FWStringView EatAlphas(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatDigits(FStringView& str);
PPE_CORE_API FWStringView EatDigits(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatXDigits(FStringView& str);
PPE_CORE_API FWStringView EatXDigits(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatIdentifier(FStringView& str);
PPE_CORE_API FWStringView EatIdentifier(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatOctals(FStringView& str);
PPE_CORE_API FWStringView EatOctals(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatPrints(FStringView& str);
PPE_CORE_API FWStringView EatPrints(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatSpaces(FStringView& str);
PPE_CORE_API FWStringView EatSpaces(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatWhile(FStringView& str, char ch);
PPE_CORE_API FWStringView EatWhile(FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatWhile(FStringView& str, const FStringView& multiple);
PPE_CORE_API FWStringView EatWhile(FWStringView& wstr, const FWStringView& wmultiple);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatUntil(FStringView& str, char ch);
PPE_CORE_API FWStringView EatUntil(FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView EatUntil(FStringView& str, const FStringView& multiple);
PPE_CORE_API FWStringView EatUntil(FWStringView& wstr, const FWStringView& wmultiple);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FStringView TrimStart(const FStringView& str, char ch);
PPE_CORE_API FWStringView TrimStart(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView TrimStart(const FStringView& str, const FStringView& chars);
PPE_CORE_API FWStringView TrimStart(const FWStringView& wstr, const FWStringView& wchars);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView TrimEnd(const FStringView& str, char ch);
PPE_CORE_API FWStringView TrimEnd(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView TrimEnd(const FStringView& str, const FStringView& chars);
PPE_CORE_API FWStringView TrimEnd(const FWStringView& wstr, const FWStringView& wchars);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView Trim(const FStringView& str, char ch);
PPE_CORE_API FWStringView Trim(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView Trim(const FStringView& str, const FStringView& chars);
PPE_CORE_API FWStringView Trim(const FWStringView& wstr, const FWStringView& wchars);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView Chomp(const FStringView& str);
PPE_CORE_API FWStringView Chomp(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API FStringView Strip(const FStringView& str);
PPE_CORE_API FWStringView Strip(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API int Compare(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
PPE_CORE_API int Compare(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API int CompareI(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
PPE_CORE_API int CompareI(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API bool EqualsN(const char* lhs, const char* rhs, size_t len) NOEXCEPT;
PPE_CORE_API bool EqualsNI(const char* lhs, const char* rhs, size_t len) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API bool EqualsN(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT;
PPE_CORE_API bool EqualsNI(const wchar_t* lhs, const wchar_t* rhs, size_t len) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API bool Equals(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
PPE_CORE_API bool Equals(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API bool EqualsI(const FStringView& lhs, const FStringView& rhs) NOEXCEPT;
PPE_CORE_API bool EqualsI(const FWStringView& lhs, const FWStringView& rhs) NOEXCEPT;
//----------------------------------------------------------------------------
inline bool EqualsI(char lhs, char rhs) NOEXCEPT { return (ToUpper(lhs) == ToUpper(rhs)); }
inline bool EqualsI(wchar_t lhs, wchar_t rhs) NOEXCEPT { return (ToUpper(lhs) == ToUpper(rhs)); }
//----------------------------------------------------------------------------
inline bool Equals(char lhs, char rhs, ECase cmp) NOEXCEPT { return (ECase::Insensitive == cmp ? ToUpper(lhs) == ToUpper(rhs) : lhs == rhs); }
inline bool Equals(wchar_t lhs, wchar_t rhs, ECase cmp) NOEXCEPT { return (ECase::Insensitive == cmp ? ToUpper(lhs) == ToUpper(rhs) : lhs == rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API bool StartsWith(const FStringView& str, const FStringView& prefix);
PPE_CORE_API bool StartsWith(const FWStringView& wstr, const FWStringView& wprefix);
//----------------------------------------------------------------------------
PPE_CORE_API bool StartsWithI(const FStringView& str, const FStringView& prefix);
PPE_CORE_API bool StartsWithI(const FWStringView& wstr, const FWStringView& wprefix);
//----------------------------------------------------------------------------
PPE_CORE_API bool EndsWith(const FStringView& str, const FStringView& suffix);
PPE_CORE_API bool EndsWith(const FWStringView& wstr, const FWStringView& wsuffix);
//----------------------------------------------------------------------------
PPE_CORE_API bool EndsWithI(const FStringView& str, const FStringView& suffix);
PPE_CORE_API bool EndsWithI(const FWStringView& wstr, const FWStringView& wsuffix);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API bool Split(FStringView& str, char separator, FStringView& slice);
PPE_CORE_API bool Split(FWStringView& wstr, wchar_t separator, FWStringView& slice);
//----------------------------------------------------------------------------
PPE_CORE_API bool Split(FStringView& str, const FStringView& separators, FStringView& slice);
PPE_CORE_API bool Split(FWStringView& wstr, const FWStringView& separators, FWStringView& slice);
//----------------------------------------------------------------------------
PPE_CORE_API bool SplitR(FStringView& str, char separator, FStringView& slice);
PPE_CORE_API bool SplitR(FWStringView& wstr, wchar_t separator, FWStringView& slice);
//----------------------------------------------------------------------------
PPE_CORE_API bool SplitR(FStringView& str, const FStringView& separators, FStringView& slice);
PPE_CORE_API bool SplitR(FWStringView& wstr, const FWStringView& separators, FWStringView& slice);
//----------------------------------------------------------------------------
PPE_CORE_API bool SplitNth(FStringView& str, char separator, FStringView& slice, size_t nth);
PPE_CORE_API bool SplitNth(FWStringView& wstr, wchar_t separator, FWStringView& slice, size_t nth);
//----------------------------------------------------------------------------
PPE_CORE_API bool SplitNth(FStringView& str, const FStringView& separators, FStringView& slice, size_t nth);
PPE_CORE_API bool SplitNth(FWStringView& wstr, const FWStringView& separators, FWStringView& slice, size_t nth);
//----------------------------------------------------------------------------
PPE_CORE_API bool SplitNthR(FStringView& str, char separator, FStringView& slice, size_t nth);
PPE_CORE_API bool SplitNthR(FWStringView& wstr, wchar_t separator, FWStringView& slice, size_t nth);
//----------------------------------------------------------------------------
PPE_CORE_API bool SplitNthR(FStringView& str, const FStringView& separators, FStringView& slice, size_t nth);
PPE_CORE_API bool SplitNthR(FWStringView& wstr, const FWStringView& separators, FWStringView& slice, size_t nth);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> Slice(const TBasicStringView<_Char>& str, size_t offset, size_t count) {
    return str.SubRange(offset, count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API bool Atoi(i32* dst, const FStringView& str, size_t base);
PPE_CORE_API bool Atoi(u32* dst, const FStringView& str, size_t base);
PPE_CORE_API bool Atoi(i64* dst, const FStringView& str, size_t base);
PPE_CORE_API bool Atoi(u64* dst, const FStringView& str, size_t base);
//----------------------------------------------------------------------------
PPE_CORE_API bool Atoi(i32* dst, const FWStringView& wstr, size_t base);
PPE_CORE_API bool Atoi(u32* dst, const FWStringView& wstr, size_t base);
PPE_CORE_API bool Atoi(i64* dst, const FWStringView& wstr, size_t base);
PPE_CORE_API bool Atoi(u64* dst, const FWStringView& wstr, size_t base);
//----------------------------------------------------------------------------
PPE_CORE_API bool Atof(float* dst, const FStringView& str);
PPE_CORE_API bool Atod(double* dst, const FStringView& str);
//----------------------------------------------------------------------------
PPE_CORE_API bool Atof(float* dst, const FWStringView& wstr);
PPE_CORE_API bool Atod(double* dst, const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API bool WildMatch(const FStringView& pattern, const FStringView& str);
PPE_CORE_API bool WildMatch(const FWStringView& pattern, const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API bool WildMatchI(const FStringView& pattern, const FStringView& str);
PPE_CORE_API bool WildMatchI(const FWStringView& pattern, const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API size_t LevenshteinDistance(const FStringView& lhs, const FStringView& rhs);
PPE_CORE_API size_t LevenshteinDistance(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
PPE_CORE_API size_t LevenshteinDistanceI(const FStringView& lhs, const FStringView& rhs);
PPE_CORE_API size_t LevenshteinDistanceI(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API size_t Copy(const TMemoryView<char>& dst, const FStringView& src);
PPE_CORE_API size_t Copy(const TMemoryView<wchar_t>& dst, const FWStringView& src);
//----------------------------------------------------------------------------
PPE_CORE_API const char* NullTerminated(const TMemoryView<char>& dst, const FStringView& src);
PPE_CORE_API const wchar_t* NullTerminated(const TMemoryView<wchar_t>& dst, const FWStringView& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API hash_t hash_string(const FStringView& str);
PPE_CORE_API hash_t hash_string(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_CORE_API hash_t hash_stringI(const FStringView& str);
PPE_CORE_API hash_t hash_stringI(const FWStringView& wstr);
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
    CONSTEXPR bool operator ()(const _Char& lhs, const _Char& rhs) const NOEXCEPT { return lhs == rhs; }
};
template <typename _Char>
struct TCharEqualTo<_Char, ECase::Insensitive> {
    CONSTEXPR bool operator ()(const _Char& lhs, const _Char& rhs) const NOEXCEPT { return ToLower(lhs) == ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TCharLess {
    CONSTEXPR bool operator ()(const _Char& lhs, const _Char& rhs) const NOEXCEPT { return lhs < rhs; }
};
template <typename _Char>
struct TCharLess<_Char, ECase::Insensitive> {
    CONSTEXPR bool operator ()(const _Char& lhs, const _Char& rhs) const NOEXCEPT { return ToLower(lhs) < ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TCharCase {
    CONSTEXPR _Char operator ()(const _Char& ch) const NOEXCEPT { return ch; }
};
template <typename _Char>
struct TCharCase<_Char, ECase::Insensitive> {
    CONSTEXPR _Char operator ()(const _Char& ch) const NOEXCEPT { return ToLower(ch); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
CONSTEXPR auto MakeCaseInsensitive(const TBasicStringView<_Char>& str) NOEXCEPT {
    return MakeOutputIterable(str.data(), str.data() + str.size(), TCharCase<_Char, ECase::Insensitive>{});
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
CONSTEXPR auto MakeCaseInsensitive(const _Char (&staticChars)[_Dim]) NOEXCEPT {
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
    bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT {
        return Equals(lhs, rhs);
    }
};
template <typename _Char>
struct TStringViewEqualTo<_Char, ECase::Insensitive> {
    bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT {
        return EqualsI(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewLess {
    bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT {
        return (Compare(lhs, rhs) < 0);
    }
};
template <typename _Char>
struct TStringViewLess<_Char, ECase::Insensitive> {
    bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const NOEXCEPT {
        return (CompareI(lhs, rhs) < 0);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewHasher {
    hash_t operator ()(const TBasicStringView<_Char>& str) const NOEXCEPT {
        return hash_string(str);
    }
};
template <typename _Char>
struct TStringViewHasher<_Char, ECase::Insensitive> {
    hash_t operator ()(const TBasicStringView<_Char>& str) const NOEXCEPT {
        return hash_stringI(str);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
