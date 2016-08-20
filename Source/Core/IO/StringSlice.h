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
class BasicStringSlice : public MemoryView< typename std::add_const<_Char>::type > {
public:
    typedef MemoryView< typename std::add_const<_Char>::type > parent_type;

    using parent_type::parent_type;
    using parent_type::operator=;

    BasicStringSlice() = default;

    BasicStringSlice(const BasicStringSlice& other) = default;
    BasicStringSlice& operator =(const BasicStringSlice& other) = default;

    BasicStringSlice(BasicStringSlice&& rvalue) = default;
    BasicStringSlice& operator =(BasicStringSlice&& rvalue) = default;

    BasicStringSlice(const parent_type& other) : parent_type(other) {}
    BasicStringSlice& operator =(const parent_type& other) { parent_type::operator =(other); return *this; }

    BasicStringSlice(parent_type&& rvalue) : parent_type(std::move(rvalue)) {}
    BasicStringSlice& operator =(parent_type&& rvalue) { parent_type::operator =(std::move(rvalue)); return *this; }

    template <size_t _Dim>
    BasicStringSlice(const _Char (&staticChars)[_Dim])
        : parent_type(staticChars, _Dim - 1/* assume null terminated string */) {
        static_assert(_Dim, "invalid string");
        Assert(not staticChars[_Dim - 1]);
    }
};
//----------------------------------------------------------------------------
typedef BasicStringSlice<char>      StringSlice;
typedef BasicStringSlice<wchar_t>   WStringSlice;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator >
FORCE_INLINE BasicStringSlice<_Char> MakeStringSlice(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return BasicStringSlice<_Char>(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim>
FORCE_INLINE BasicStringSlice<_Char> MakeStringSlice(const _Char(&cstr)[_Dim]) {
    return BasicStringSlice<_Char>(cstr);
}
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE BasicStringSlice<_Char> MakeStringSlice(const MemoryView<const _Char>& view) {
    return BasicStringSlice<_Char>(view);
}
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE BasicStringSlice<_Char> MakeStringSlice(const MemoryView<_Char>& view) {
    return BasicStringSlice<_Char>(view.AddConst());
}
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE BasicStringSlice<_Char> MakeStringSlice(const BasicStringSlice<_Char>& slice) {
    return slice;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline size_t Length(const char* string) { return ::strlen(string); }
inline size_t Length(const wchar_t* string) { return ::wcslen(string); }
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE BasicStringSlice<_Char> MakeStringSlice(const _Char* cstr, Meta::noinit_tag ) {
    return (nullptr != cstr)
        ? BasicStringSlice<_Char>(cstr, Length(cstr))
        : BasicStringSlice<_Char>();
}
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
FORCE_INLINE bool IsPrint(char ch) { return 0 != std::isprint(int(ch)); }
FORCE_INLINE bool IsPrint(wchar_t wch) { return 0 != std::iswprint(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsPunct(char ch) { return 0 != std::ispunct(int(ch)); }
FORCE_INLINE bool IsPunct(wchar_t wch) { return 0 != std::iswpunct(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsSpace(char ch) { return ( ch == ' ' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' ); }
FORCE_INLINE bool IsSpace(wchar_t wch) { return 0 != std::iswdigit(wch); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IsAlnum(const StringSlice& str);
bool IsAlnum(const WStringSlice& wstr);
//----------------------------------------------------------------------------
bool IsAlpha(const StringSlice& str);
bool IsAlpha(const WStringSlice& wstr);
//----------------------------------------------------------------------------
bool IsDigit(const StringSlice& str);
bool IsDigit(const WStringSlice& wstr);
//----------------------------------------------------------------------------
bool IsXDigit(const StringSlice& str);
bool IsXDigit(const WStringSlice& wstr);
//----------------------------------------------------------------------------
bool IsPrint(const StringSlice& str);
bool IsPrint(const WStringSlice& wstr);
//----------------------------------------------------------------------------
bool IsSpace(const StringSlice& str);
bool IsSpace(const WStringSlice& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice EatAlnums(StringSlice& str);
WStringSlice EatAlnums(WStringSlice& wstr);
//----------------------------------------------------------------------------
StringSlice EatAlphas(StringSlice& str);
WStringSlice EatAlphas(WStringSlice& wstr);
//----------------------------------------------------------------------------
StringSlice EatDigits(StringSlice& str);
WStringSlice EatDigits(WStringSlice& wstr);
//----------------------------------------------------------------------------
StringSlice EatXDigits(StringSlice& str);
WStringSlice EatXDigits(WStringSlice& wstr);
//----------------------------------------------------------------------------
StringSlice EatPrints(StringSlice& str);
WStringSlice EatPrints(WStringSlice& wstr);
//----------------------------------------------------------------------------
StringSlice EatSpaces(StringSlice& str);
WStringSlice EatSpaces(WStringSlice& wstr);
//----------------------------------------------------------------------------
StringSlice Chomp(const StringSlice& str);
WStringSlice Chomp(const WStringSlice& wstr);
//----------------------------------------------------------------------------
StringSlice Strip(const StringSlice& str);
WStringSlice Strip(const WStringSlice& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const StringSlice& lhs, const StringSlice& rhs);
int Compare(const WStringSlice& lhs, const WStringSlice& rhs);
//----------------------------------------------------------------------------
int CompareI(const StringSlice& lhs, const StringSlice& rhs);
int CompareI(const WStringSlice& lhs, const WStringSlice& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool Equals(const StringSlice& lhs, const StringSlice& rhs) { return (0 == Compare(lhs, rhs)); }
inline bool Equals(const WStringSlice& lhs, const WStringSlice& rhs) { return (0 == Compare(lhs, rhs)); }
//----------------------------------------------------------------------------
inline bool EqualsI(const StringSlice& lhs, const StringSlice& rhs) { return (0 == CompareI(lhs, rhs)); }
inline bool EqualsI(const WStringSlice& lhs, const WStringSlice& rhs) { return (0 == CompareI(lhs, rhs)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Split(StringSlice& str, char separator, StringSlice& slice);
bool Split(WStringSlice& wstr, wchar_t separator, WStringSlice& slice);
//----------------------------------------------------------------------------
bool Split(StringSlice& str, const StringSlice& separators, StringSlice& slice);
bool Split(WStringSlice& wstr, const WStringSlice& separators, WStringSlice& slice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const StringSlice& str, size_t base);
bool Atoi64(i64* dst, const StringSlice& str, size_t base);
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const WStringSlice& wstr, size_t base);
bool Atoi64(i64* dst, const WStringSlice& wstr, size_t base);
//----------------------------------------------------------------------------
template <typename _Char>
bool Atoi(intptr_t* dst, const BasicStringSlice<_Char>& str, size_t base) {
#ifdef ARCH_X64
    STATIC_ASSERT(sizeof(intptr_t) == sizeof(i64));
    return Atoi64(dst, str, base);
#else
    STATIC_ASSERT(sizeof(intptr_t) == sizeof(i32));
    return Atoi32(dst, str, base);
#endif
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const StringSlice& str);
bool Atod(double* dst, const StringSlice& str);
//----------------------------------------------------------------------------
bool Atof(float* dst, const WStringSlice& wstr);
bool Atod(double* dst, const WStringSlice& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const StringSlice& pattern, const StringSlice& str);
bool WildMatch(const WStringSlice& pattern, const WStringSlice& wstr);
//----------------------------------------------------------------------------
bool WildMatchI(const StringSlice& pattern, const StringSlice& str);
bool WildMatchI(const WStringSlice& pattern, const WStringSlice& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t Copy(const MemoryView<char>& dst, const StringSlice& src);
size_t Copy(const MemoryView<wchar_t>& dst, const WStringSlice& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const StringSlice& str);
hash_t hash_string(const WStringSlice& wstr);
//----------------------------------------------------------------------------
hash_t hash_stringI(const StringSlice& str);
hash_t hash_stringI(const WStringSlice& wstr);
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
struct StringSliceEqualTo {
    bool operator ()(const BasicStringSlice<_Char>& lhs, const BasicStringSlice<_Char>& rhs) const {
        return (0 == Compare(lhs, rhs));
    }
};
template <typename _Char>
struct StringSliceEqualTo<_Char, Case::Insensitive> {
    bool operator ()(const BasicStringSlice<_Char>& lhs, const BasicStringSlice<_Char>& rhs) const {
        return (0 == CompareI(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive>
struct StringSliceLess {
    bool operator ()(const BasicStringSlice<_Char>& lhs, const BasicStringSlice<_Char>& rhs) const {
        return (-1 == Compare(lhs, rhs));
    }
};
template <typename _Char>
struct StringSliceLess<_Char, Case::Insensitive> {
    bool operator ()(const BasicStringSlice<_Char>& lhs, const BasicStringSlice<_Char>& rhs) const {
        return (-1 == CompareI(lhs, rhs));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, Case _Sensitive>
struct StringSliceHasher {
    size_t operator ()(const BasicStringSlice<_Char>& str) const {
        return hash_string(str);
    }
};
template <typename _Char>
struct StringSliceHasher<_Char, Case::Insensitive> {
    size_t operator ()(const BasicStringSlice<_Char>& str) const {
        return hash_stringI(str);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const StringSlice& slice);
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const WStringSlice& wslice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
