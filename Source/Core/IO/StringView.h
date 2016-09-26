#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryView.h"

#include <cctype>
#include <cwctype>
#include <locale>
#include <iosfwd>
#include <string>
#include <string.h>
#include <wchar.h>
#include <xhash>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class Case : bool {
    Sensitive   = true,
    Insensitive = false,
};
//----------------------------------------------------------------------------
template <typename _Char>
class BasicStringView : public MemoryView< typename std::add_const<_Char>::type > {
public:
    typedef MemoryView< typename std::add_const<_Char>::type > parent_type;

    using typename parent_type::iterator;
    using parent_type::parent_type;
    using parent_type::operator=;

    BasicStringView() = default;

    BasicStringView(const BasicStringView& other) = default;
    BasicStringView& operator =(const BasicStringView& other) = default;

    BasicStringView(BasicStringView&& rvalue) = default;
    BasicStringView& operator =(BasicStringView&& rvalue) = default;

    BasicStringView(const parent_type& other) : parent_type(other) {}
    BasicStringView& operator =(const parent_type& other) { parent_type::operator =(other); return *this; }

    BasicStringView(parent_type&& rvalue) : parent_type(std::move(rvalue)) {}
    BasicStringView& operator =(parent_type&& rvalue) { parent_type::operator =(std::move(rvalue)); return *this; }

    template <size_t _Dim>
    BasicStringView(const _Char (&staticChars)[_Dim])
        : parent_type(staticChars, _Dim - 1/* assume null terminated string */) {
        static_assert(_Dim, "invalid string");
        Assert(not staticChars[_Dim - 1]);
    }

    template <size_t _Dim>
    void ToNullTerminatedCStr(_Char (&dst)[_Dim]) const {
        Assert(_Dim > size());
        CopyTo(dst);
        dst[size()] = _Char(0);
    }
};
//----------------------------------------------------------------------------
typedef BasicStringView<char>      StringView;
typedef BasicStringView<wchar_t>   WStringView;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator >
FORCE_INLINE BasicStringView<_Char> MakeStringView(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return BasicStringView<_Char>(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
FORCE_INLINE BasicStringView<_Char> MakeStringView(const _Char(&cstr)[_Dim]) {
    return BasicStringView<_Char>(cstr);
}
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE BasicStringView<_Char> MakeStringView(const MemoryView<const _Char>& view) {
    return BasicStringView<_Char>(view);
}
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE BasicStringView<_Char> MakeStringView(const MemoryView<_Char>& view) {
    return BasicStringView<_Char>(view.AddConst());
}
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE BasicStringView<_Char> MakeStringView(const BasicStringView<_Char>& slice) {
    return slice;
}
//----------------------------------------------------------------------------
template <typename _It>
typename std::enable_if<
    Meta::is_iterator_of<_It, char>::value,
    StringView
>::type MakeStringView(_It first, _It last) {
    typedef std::iterator_traits<_It> traits_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return StringView(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
template <typename _It>
typename std::enable_if<
    Meta::is_iterator_of<_It, wchar_t>::value,
    WStringView
>::type MakeStringView(_It first, _It last) {
    typedef std::iterator_traits<_It> traits_type;
    STATIC_ASSERT(std::is_same<typename traits_type::iterator_category, std::random_access_iterator_tag>::value);
    return WStringView(std::addressof(*first), std::distance(first, last));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline size_t Length(const char* string) { return ::strlen(string); }
inline size_t Length(const wchar_t* string) { return ::wcslen(string); }
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE BasicStringView<_Char> MakeStringView(const _Char* cstr, Meta::noinit_tag ) {
    return (nullptr != cstr)
        ? BasicStringView<_Char>(cstr, Length(cstr))
        : BasicStringView<_Char>();
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
StringView::iterator StrChr(const StringView& str, char ch);
StringView::reverse_iterator StrRChr(const StringView& str, char ch);
//----------------------------------------------------------------------------
WStringView::iterator StrChr(const WStringView& wstr, wchar_t wch);
WStringView::reverse_iterator StrRChr(const WStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
StringView::iterator StrStr(const StringView& str, const StringView& firstOccurence);
WStringView::iterator StrStr(const WStringView& wstr, const WStringView& firstOccurence);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline char ToLower(char ch) { return ( ( ch >= 'A' ) && ( ch <= 'Z' ) ) ? 'a' + ( ch - 'A' ) : ch; }
inline char ToUpper(char ch) { return ( ( ch >= 'a' ) && ( ch <= 'z' ) ) ? 'A' + ( ch - 'a' ) : ch; }
//----------------------------------------------------------------------------
inline wchar_t ToLower(wchar_t wch) { return std::towlower(wch); }
inline wchar_t ToUpper(wchar_t wch) { return std::towupper(wch); }
//----------------------------------------------------------------------------
template <typename _Char> void InplaceToLower(_Char& ch) { ch = ToLower(ch); }
template <typename _Char> void InplaceToUpper(_Char& ch) { ch = ToUpper(ch); }
//----------------------------------------------------------------------------
template <typename _Char> void InplaceToLower(const MemoryView<_Char>& str) { for (_Char& ch : str) InplaceToLower(ch); }
template <typename _Char> void InplaceToUpper(const MemoryView<_Char>& str) { for (_Char& ch : str) InplaceToUpper(ch); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE bool IsAlpha(char ch) { return ( ( ch >= 'a' ) && ( ch <= 'z' ) ) || ( ( ch >= 'A' ) && ( ch <= 'Z' ) ); }
FORCE_INLINE bool IsAlpha(wchar_t wch) { return 0 != std::iswalpha(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsDigit(char ch) { return ( ( ch >= '0' ) && ( ch <= '9' ) ); }
FORCE_INLINE bool IsDigit(wchar_t wch) { return 0 != std::iswdigit(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsEndLine(char ch) { return ( ( ch == '\r' ) || ( ch == '\n' ) ); }
FORCE_INLINE bool IsEndLine(wchar_t wch) { return ( ( wch == L'\r' ) || ( wch == L'\n' ) ); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsXDigit(char ch) { return IsDigit(ch) || ( ch >= 'a' && ch <= 'f' ) || ( ch >= 'A' && ch <= 'F' ); }
FORCE_INLINE bool IsXDigit(wchar_t wch) { return 0 != std::iswxdigit(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsAlnum(char ch) { return IsAlpha(ch) || IsDigit(ch); }
FORCE_INLINE bool IsAlnum(wchar_t wch) { return 0 != std::iswalnum(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsPrint(char ch) { return 0 != std::isprint(int(unsigned char(ch))); }
FORCE_INLINE bool IsPrint(wchar_t wch) { return 0 != std::iswprint(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsPunct(char ch) { return 0 != std::ispunct(int(unsigned char(ch))); }
FORCE_INLINE bool IsPunct(wchar_t wch) { return 0 != std::iswpunct(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsSpace(char ch) { return ( ch == ' ' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' ); }
FORCE_INLINE bool IsSpace(wchar_t wch) { return 0 != std::iswdigit(wch); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IsAlnum(const StringView& str);
bool IsAlnum(const WStringView& wstr);
//----------------------------------------------------------------------------
bool IsAlpha(const StringView& str);
bool IsAlpha(const WStringView& wstr);
//----------------------------------------------------------------------------
bool IsDigit(const StringView& str);
bool IsDigit(const WStringView& wstr);
//----------------------------------------------------------------------------
bool IsXDigit(const StringView& str);
bool IsXDigit(const WStringView& wstr);
//----------------------------------------------------------------------------
bool IsPrint(const StringView& str);
bool IsPrint(const WStringView& wstr);
//----------------------------------------------------------------------------
bool IsSpace(const StringView& str);
bool IsSpace(const WStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView EatAlnums(StringView& str);
WStringView EatAlnums(WStringView& wstr);
//----------------------------------------------------------------------------
StringView EatAlphas(StringView& str);
WStringView EatAlphas(WStringView& wstr);
//----------------------------------------------------------------------------
StringView EatDigits(StringView& str);
WStringView EatDigits(WStringView& wstr);
//----------------------------------------------------------------------------
StringView EatXDigits(StringView& str);
WStringView EatXDigits(WStringView& wstr);
//----------------------------------------------------------------------------
StringView EatPrints(StringView& str);
WStringView EatPrints(WStringView& wstr);
//----------------------------------------------------------------------------
StringView EatSpaces(StringView& str);
WStringView EatSpaces(WStringView& wstr);
//----------------------------------------------------------------------------
StringView Chomp(const StringView& str);
WStringView Chomp(const WStringView& wstr);
//----------------------------------------------------------------------------
StringView Strip(const StringView& str);
WStringView Strip(const WStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const StringView& lhs, const StringView& rhs);
int Compare(const WStringView& lhs, const WStringView& rhs);
//----------------------------------------------------------------------------
int CompareI(const StringView& lhs, const StringView& rhs);
int CompareI(const WStringView& lhs, const WStringView& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool Equals(const StringView& lhs, const StringView& rhs) { return (0 == Compare(lhs, rhs)); }
inline bool Equals(const WStringView& lhs, const WStringView& rhs) { return (0 == Compare(lhs, rhs)); }
//----------------------------------------------------------------------------
inline bool EqualsI(const StringView& lhs, const StringView& rhs) { return (0 == CompareI(lhs, rhs)); }
inline bool EqualsI(const WStringView& lhs, const WStringView& rhs) { return (0 == CompareI(lhs, rhs)); }
//----------------------------------------------------------------------------
inline bool EqualsI(char lhs, char rhs) { return (ToUpper(lhs) == ToUpper(rhs)); }
inline bool EqualsI(wchar_t lhs, wchar_t rhs) { return (ToUpper(lhs) == ToUpper(rhs)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Split(StringView& str, char separator, StringView& slice);
bool Split(WStringView& wstr, wchar_t separator, WStringView& slice);
//----------------------------------------------------------------------------
bool Split(StringView& str, const StringView& separators, StringView& slice);
bool Split(WStringView& wstr, const WStringView& separators, WStringView& slice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const StringView& str, size_t base);
bool Atoi64(i64* dst, const StringView& str, size_t base);
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const WStringView& wstr, size_t base);
bool Atoi64(i64* dst, const WStringView& wstr, size_t base);
//----------------------------------------------------------------------------
template <typename _Char>
bool Atoi(intptr_t* dst, const BasicStringView<_Char>& str, size_t base) {
#ifdef ARCH_X64
    STATIC_ASSERT(sizeof(intptr_t) == sizeof(i64));
    return Atoi64(dst, str, base);
#else
    STATIC_ASSERT(sizeof(intptr_t) == sizeof(i32));
    return Atoi32(dst, str, base);
#endif
}
//----------------------------------------------------------------------------
template <typename _Char>
bool Atoi(size_t* dst, const BasicStringView<_Char>& str, size_t base) {
    return Atoi((intptr_t*)dst, str, base);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const StringView& str);
bool Atod(double* dst, const StringView& str);
//----------------------------------------------------------------------------
bool Atof(float* dst, const WStringView& wstr);
bool Atod(double* dst, const WStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const StringView& pattern, const StringView& str);
bool WildMatch(const WStringView& pattern, const WStringView& wstr);
//----------------------------------------------------------------------------
bool WildMatchI(const StringView& pattern, const StringView& str);
bool WildMatchI(const WStringView& pattern, const WStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t Copy(const MemoryView<char>& dst, const StringView& src);
size_t Copy(const MemoryView<wchar_t>& dst, const WStringView& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const StringView& str);
hash_t hash_string(const WStringView& wstr);
//----------------------------------------------------------------------------
hash_t hash_stringI(const StringView& str);
hash_t hash_stringI(const WStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive>
struct CharEqualTo : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return lhs == rhs; }
};
template <typename _Char>
struct CharEqualTo<_Char, Case::Insensitive> : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return ToLower(lhs) == ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive>
struct CharLess : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return lhs < rhs; }
};
template <typename _Char>
struct CharLess<_Char, Case::Insensitive> : public std::binary_function<const _Char, const _Char, bool>{
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return ToLower(lhs) < ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive>
struct CharCase : public std::unary_function<const _Char, _Char> {
    _Char operator ()(const _Char& ch) const { return ch; }
};
template <typename _Char>
struct CharCase<_Char, Case::Insensitive> : public std::unary_function<const _Char, _Char> {
    _Char operator ()(const _Char& ch) const { return ToLower(ch); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive>
struct StringViewEqualTo {
    bool operator ()(const BasicStringView<_Char>& lhs, const BasicStringView<_Char>& rhs) const {
        return (0 == Compare(lhs, rhs));
    }
};
template <typename _Char>
struct StringViewEqualTo<_Char, Case::Insensitive> {
    bool operator ()(const BasicStringView<_Char>& lhs, const BasicStringView<_Char>& rhs) const {
        return (0 == CompareI(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive>
struct StringViewLess {
    bool operator ()(const BasicStringView<_Char>& lhs, const BasicStringView<_Char>& rhs) const {
        return (-1 == Compare(lhs, rhs));
    }
};
template <typename _Char>
struct StringViewLess<_Char, Case::Insensitive> {
    bool operator ()(const BasicStringView<_Char>& lhs, const BasicStringView<_Char>& rhs) const {
        return (-1 == CompareI(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive>
struct StringViewHasher {
    size_t operator ()(const BasicStringView<_Char>& str) const {
        return hash_string(str);
    }
};
template <typename _Char>
struct StringViewHasher<_Char, Case::Insensitive> {
    size_t operator ()(const BasicStringView<_Char>& str) const {
        return hash_stringI(str);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const StringView& slice);
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const WStringView& wslice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
