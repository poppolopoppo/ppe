#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/Stream.h"
#include "Core/Memory/MemoryView.h"

#include <cctype>
#include <cwctype>
#include <locale>
#include <iosfwd>
#include <string>
#include <string.h>
#include <wchar.h>
#include <xhash>

extern template class std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
extern template class std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<char>>;

extern template class std::basic_string<char, std::char_traits<char>, ALLOCATOR(String, char)>;
extern template class std::basic_string<wchar_t, std::char_traits<wchar_t>, ALLOCATOR(String, wchar_t)>;

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class CaseSensitive : bool {
    True    = true,
    False   = false,
};
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Traits = std::char_traits<_Char>,
    typename _Allocator = ALLOCATOR(String, _Char)
>
using BasicString = std::basic_string<_Char, _Traits, _Allocator>;
//----------------------------------------------------------------------------
INSTANTIATE_CLASS_TYPEDEF(String,   BasicString<char>);
INSTANTIATE_CLASS_TYPEDEF(WString,  BasicString<wchar_t>);
//----------------------------------------------------------------------------
template <typename _Char> struct DefaultString {};
template <> struct DefaultString<char> { typedef String type; };
template <> struct DefaultString<wchar_t> { typedef WString type; };
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const char* cstr, size_t length);
hash_t hash_string(const wchar_t* wcstr, size_t length);
//----------------------------------------------------------------------------
hash_t hash_stringI(const char* cstr, size_t length);
hash_t hash_stringI(const wchar_t* wcstr, size_t length);
//----------------------------------------------------------------------------
inline hash_t hash_value(const String& str) { return hash_string(str.c_str(), str.size()); }
inline hash_t hash_value(const WString& wstr) { return hash_string(wstr.c_str(), wstr.size()); }
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator>
hash_t hash_value(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return hash_string(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline size_t Length(const char* string) { return ::strlen(string); }
//----------------------------------------------------------------------------
inline size_t Length(const wchar_t* string) { return ::wcslen(string); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline void Copy(char* dst, size_t capacity, const char* src) { ::strcpy_s(dst, capacity, src); }
//----------------------------------------------------------------------------
inline void Copy(wchar_t* dst, size_t capacity, const wchar_t* src) { ::wcscpy_s(dst, capacity, src); }
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity >
void Copy(_Char(&dst)[_Capacity], const _Char* src) { Copy(dst, _Capacity, src); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline int Compare(const char* lhs, const char* rhs) { return ::strcmp(lhs, rhs); }
//----------------------------------------------------------------------------
inline int Compare(const wchar_t* lhs, const wchar_t* rhs) { return ::wcscmp(lhs, rhs); }
//----------------------------------------------------------------------------
inline int CompareI(const char* lhs, const char* rhs) { return ::_strcmpi(lhs, rhs); }
//----------------------------------------------------------------------------
inline int CompareI(const wchar_t* lhs, const wchar_t* rhs) { return ::_wcsicmp(lhs, rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline int CompareN(const char* lhs, const char* rhs, size_t length) { return ::strncmp(lhs, rhs, length); }
//----------------------------------------------------------------------------
inline int CompareN(const wchar_t* lhs, const wchar_t* rhs, size_t length) { return ::wcsncmp(lhs, rhs, length); }
//----------------------------------------------------------------------------
inline int CompareNI(const char* lhs, const char* rhs, size_t length) { return ::_strnicmp(lhs, rhs, length); }
//----------------------------------------------------------------------------
inline int CompareNI(const wchar_t* lhs, const wchar_t* rhs, size_t length) { return ::_wcsnicmp(lhs, rhs, length); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline const char *StrChr(const char* cstr, char ch) { return strchr(cstr, ch); }
//----------------------------------------------------------------------------
inline const char *StrRChr(const char* cstr, char ch) { return strrchr(cstr, ch); }
//----------------------------------------------------------------------------
inline const wchar_t *StrChr(const wchar_t* wcstr, wchar_t wch) { return wcschr(wcstr, wch); }
//----------------------------------------------------------------------------
inline const wchar_t *StrRChr(const wchar_t* wcstr, wchar_t wch) { return wcsrchr(wcstr, wch); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline const char *StrStr(const char* cstr, const char* firstOccurence) { return strstr(cstr, firstOccurence); }
//----------------------------------------------------------------------------
inline const wchar_t *StrStr(const wchar_t* wcstr, const wchar_t* firstOccurence) { return wcsstr(wcstr, firstOccurence); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool StartsWith(const char* cstr, const char *other) { return 0 == strncmp(cstr, other, Length(other)); }
//----------------------------------------------------------------------------
template <size_t _Capacity>
bool StartsWith(const char* cstr, const char (&other)[_Capacity]) { return 0 == strncmp(cstr, other, _Capacity); }
//----------------------------------------------------------------------------
inline bool StartsWith(const wchar_t* wcstr, const wchar_t *other) { return 0 == wcsncmp(wcstr, other, Length(other)); }
//----------------------------------------------------------------------------
template <size_t _Capacity>
bool StartsWith(const wchar_t* wcstr, const wchar_t (&other)[_Capacity]) { return 0 == wcsncmp(wcstr, other, _Capacity); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const char *pattern, const char *cstr);
//----------------------------------------------------------------------------
bool WildMatch(const wchar_t *wpattern, const wchar_t *wcstr);
//----------------------------------------------------------------------------
bool WildMatchI(const char *pattern, const char *cstr);
//----------------------------------------------------------------------------
bool WildMatchI(const wchar_t *wpattern, const wchar_t *wcstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Base, typename T>
bool Atoi(T *dst, const char *cstr, size_t length);
//----------------------------------------------------------------------------
template <size_t _Base, typename T, size_t _Capacity>
bool Atoi(T *dst, const char (&cstr)[_Capacity]);
//----------------------------------------------------------------------------
template <size_t _Base, typename T>
bool Atoi(T *dst, const String& str);
//----------------------------------------------------------------------------
template <size_t _Base, typename T>
bool Atoi(T *dst, const MemoryView<const char>& strview);
//----------------------------------------------------------------------------
#define CORE_ATOIBASE_DECL(_Base) \
    extern template bool Atoi<_Base, int>(int *, const char *, size_t ); \
    extern template bool Atoi<_Base, unsigned>(unsigned *, const char *, size_t ); \
    extern template bool Atoi<_Base, i8> (i8  *, const char *, size_t ); \
    extern template bool Atoi<_Base, i16>(i16 *, const char *, size_t ); \
    extern template bool Atoi<_Base, i32>(i32 *, const char *, size_t ); \
    extern template bool Atoi<_Base, i64>(i64 *, const char *, size_t ); \
    extern template bool Atoi<_Base, u8> (u8  *, const char *, size_t ); \
    extern template bool Atoi<_Base, u16>(u16 *, const char *, size_t ); \
    extern template bool Atoi<_Base, u32>(u32 *, const char *, size_t ); \
    extern template bool Atoi<_Base, u64>(u64 *, const char *, size_t )
CORE_ATOIBASE_DECL(2);
CORE_ATOIBASE_DECL(8);
CORE_ATOIBASE_DECL(10);
CORE_ATOIBASE_DECL(16);
#undef CORE_ATOIBASE_DECL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool Atof(T *dst, const char *cstr, size_t length);
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity>
bool Atof(T *dst, const char (&cstr)[_Capacity]);
//----------------------------------------------------------------------------
template <typename T>
bool Atof(T *dst, const String& str);
//----------------------------------------------------------------------------
template <typename T>
bool Atof(T *dst, const MemoryView<const char>& strview);
//----------------------------------------------------------------------------
extern template bool Atof<float>(float *, const char *, size_t );
extern template bool Atof<double>(double *, const char *, size_t );
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline bool IsAlpha(char ch) { return ( ( ch >= 'a' ) && ( ch <= 'z' ) ) || ( ( ch >= 'A' ) && ( ch <= 'Z' ) ); }
inline bool IsAlpha(wchar_t wch) { return 0 != std::iswalpha(wch); }
//----------------------------------------------------------------------------
inline bool IsDigit(char ch) { return ( ( ch >= '0' ) && ( ch <= '9' ) ); }
inline bool IsDigit(wchar_t wch) { return 0 != std::iswdigit(wch); }
//----------------------------------------------------------------------------
inline bool IsXDigit(char ch) { return IsDigit(ch) || ( ch >= 'a' && ch <= 'f' ) || ( ch >= 'A' && ch <= 'F' ); }
inline bool IsXDigit(wchar_t wch) { return 0 != std::iswxdigit(wch); }
//----------------------------------------------------------------------------
inline bool IsAlnum(char ch) { return IsAlpha(ch) || IsDigit(ch); }
inline bool IsAlnum(wchar_t wch) { return 0 != std::iswalnum(wch); }
//----------------------------------------------------------------------------
inline bool IsPrint(char ch) { return 0 != std::isprint(ch); }
inline bool IsPrint(wchar_t wch) { return 0 != std::iswprint(wch); }
//----------------------------------------------------------------------------
inline bool IsSpace(char ch) { return ( ch == ' ' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' ); }
inline bool IsSpace(wchar_t wch) { return 0 != std::iswdigit(wch); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct CharEqualTo : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return lhs == rhs; }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct CharEqualTo<_Char, CaseSensitive::False> : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return ToLower(lhs) == ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct CharLess : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return lhs < rhs; }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct CharLess<_Char, CaseSensitive::False> : public std::binary_function<const _Char, const _Char, bool>{
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return ToLower(lhs) < ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct CharCase : public std::unary_function<const _Char, _Char> {
    _Char operator ()(const _Char& ch) const { return ch; }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct CharCase<_Char, CaseSensitive::False> : public std::unary_function<const _Char, _Char> {
    _Char operator ()(const _Char& ch) const { return ToLower(ch); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr, size_t length);
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr);
size_t ToCStr(char *dst, size_t capacity, const WString& wstr);
//----------------------------------------------------------------------------
size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr, size_t length);
size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr);
size_t ToWCStr(wchar_t *dst, size_t capacity, const String& str);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
String ToString(const wchar_t *wcstr, size_t length);
String ToString(const wchar_t *wcstr);
String ToString(const WString& wstr);
inline const String& ToString(const String& str) { return str; }
inline String ToString(const MemoryView<const char>& strview) { return String(strview.Pointer(), strview.size()); }
inline String ToString(const MemoryView<const wchar_t>& strview) { return ToString(strview.Pointer(), strview.size()); }
//----------------------------------------------------------------------------
WString ToWString(const char *cstr, size_t length);
WString ToWString(const char *cstr);
WString ToWString(const String& str);
inline const WString& ToWString(const WString& wstr) { return wstr; }
inline WString ToWString(const MemoryView<const wchar_t>& strview) { return WString(strview.Pointer(), strview.size()); }
inline WString ToWString(const MemoryView<const char>& strview) { return ToWString(strview.Pointer(), strview.size()); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
size_t ToCStr(char (&dst)[_Dim], const wchar_t *wcstr) { return ToCStr(dst, _Dim, wcstr); }
//----------------------------------------------------------------------------
template <size_t _Dim>
size_t ToWCStr(wchar_t (&dst)[_Dim], const char *cstr) { return ToWCStr(dst, _Dim, cstr); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Traits >
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const wchar_t wch);
//----------------------------------------------------------------------------
template <typename _Traits >
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const wchar_t* wstr);
//----------------------------------------------------------------------------
template <typename _Traits, typename _TraitsW, typename _Allocator >
std::basic_ostream<char, _Traits>& operator <<(
    std::basic_ostream<char, _Traits>& oss,
    const std::basic_string<wchar_t, _TraitsW, _Allocator>& wstr);
//----------------------------------------------------------------------------
template <  typename _Char, typename _Traits,
            typename _Char2, typename _Traits2,
            typename _Allocator >
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const BasicString<_Char2, _Traits2, _Allocator>& str) {
    return oss << str.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/String-inl.h"
