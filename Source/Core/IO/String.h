#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/Stream.h"
#include "Core/Memory/MemoryView.h"

#include <locale>
#include <iosfwd>
#include <string>
#include <string.h>
#include <wchar.h>
#include <xhash>

extern template std::basic_string<char, std::char_traits<char>, ALLOCATOR(String, char)>;
extern template std::basic_string<wchar_t, std::char_traits<wchar_t>, ALLOCATOR(String, wchar_t)>;

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class CaseSensitive : bool {
    True = true,
    False = false,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Traits = std::char_traits<_Char>,
    typename _Allocator = ALLOCATOR(String, _Char)
>
using BasicString = std::basic_string<_Char, _Traits, _Allocator>;
//----------------------------------------------------------------------------
using String = BasicString<char>;
using WString = BasicString<wchar_t>;
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
size_t hash_value(const char* cstr, size_t length);
//----------------------------------------------------------------------------
size_t hash_value(const wchar_t* wcstr, size_t length);
//----------------------------------------------------------------------------
size_t hash_valueI(const char* cstr, size_t length);
//----------------------------------------------------------------------------
size_t hash_valueI(const wchar_t* wcstr, size_t length);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
size_t hash_value(const std::basic_string<_Char, _Traits>& str) {
    return hash_value(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline size_t hash_value(const char* cstr) { return hash_value(cstr, Length(cstr)); }
//----------------------------------------------------------------------------
inline size_t hash_value(const wchar_t* wcstr) { return hash_value(wcstr, Length(wcstr)); }
//----------------------------------------------------------------------------
inline size_t hash_valueI(const char* cstr) { return hash_valueI(cstr, Length(cstr)); }
//----------------------------------------------------------------------------
inline size_t hash_valueI(const wchar_t* wcstr) { return hash_valueI(wcstr, Length(wcstr)); }
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
template <typename _Char, CaseSensitive _CaseSensitive>
struct StringEqualTo : public std::binary_function<const _Char*, const _Char*, bool> {
    bool operator ()(const _Char* lhs, const _Char* rhs) const { return 0 == Compare(lhs, rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct StringEqualTo<_Char, CaseSensitive::False> : public std::binary_function<const _Char*, const _Char*, bool>{
    bool operator ()(const _Char* lhs, const _Char* rhs) const { return 0 == CompareI(lhs, rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct StringLess : public std::binary_function<const _Char*, const _Char*, bool> {
    bool operator ()(const _Char* lhs, const _Char* rhs) const { return Compare(lhs, rhs) < 0; }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct StringLess<_Char, CaseSensitive::False> : public std::binary_function<const _Char*, const _Char*, bool>{
    bool operator ()(const _Char* lhs, const _Char* rhs) const { return CompareI(lhs, rhs) < 0; }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct StringHasher : public std::unary_function<const _Char*, size_t> {
    size_t operator ()(const _Char* cstr) const { return hash_value(cstr); }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct StringHasher<_Char, CaseSensitive::False> : public std::unary_function<const _Char*, size_t> {
    size_t operator ()(const _Char* cstr) const { return hash_valueI(cstr); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct CharEqualTo : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const {
        return lhs == rhs;
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct CharEqualTo<_Char, CaseSensitive::False> : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const {
        return ::tolower(lhs) == ::tolower(rhs);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, CaseSensitive _CaseSensitive>
struct CharLess : public std::binary_function<const _Char, const _Char, bool> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const {
        return lhs < rhs;
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
struct CharLess<_Char, CaseSensitive::False> : public std::binary_function<const _Char, const _Char, bool>{
    bool operator ()(const _Char& lhs, const _Char& rhs) const {
        const std::locale& locale = std::locale::classic();
        return std::tolower(lhs, locale) < std::tolower(rhs, locale);
    }
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
