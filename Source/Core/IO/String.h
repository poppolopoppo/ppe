#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/Stream.h"
#include "Core/Memory/MemoryView.h"

extern template class std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
extern template class std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<char>>;

extern template class std::basic_string<char, std::char_traits<char>, ALLOCATOR(String, char)>;
extern template class std::basic_string<wchar_t, std::char_traits<wchar_t>, ALLOCATOR(String, wchar_t)>;

extern template class std::basic_string<char, std::char_traits<char>, THREAD_LOCAL_ALLOCATOR(String, char)>;
extern template class std::basic_string<wchar_t, std::char_traits<wchar_t>, THREAD_LOCAL_ALLOCATOR(String, wchar_t)>;

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Char,
    typename _Traits = std::char_traits<_Char>,
    typename _Allocator = ALLOCATOR(String, _Char)
>
class BasicString : public std::basic_string<_Char, _Traits, _Allocator> {
public:
    typedef std::basic_string<_Char, _Traits, _Allocator> parent_type;

    using parent_type::parent_type;
    using parent_type::operator=;

    BasicString() = default;

    BasicString(const BasicString& other) = default;
    BasicString& operator =(const BasicString& other) = default;

    BasicString(BasicString&& rvalue) = default;
    BasicString& operator =(BasicString&& rvalue) = default;

    BasicString(const parent_type& other) : parent_type(other) {}
    BasicString& operator =(const parent_type& other) { parent_type::operator =(other); return *this; }

    BasicString(parent_type&& rvalue) : parent_type(std::move(rvalue)) {}
    BasicString& operator =(parent_type&& rvalue) { parent_type::operator =(std::move(rvalue)); return *this; }

    MemoryView<const _Char> MakeView() const { return MemoryView<const _Char>(c_str(), size()); }
    operator MemoryView<const _Char> () const { return MakeView(); }
};
//----------------------------------------------------------------------------
typedef BasicString<char> String;
typedef BasicString<wchar_t> WString;
//----------------------------------------------------------------------------
hash_t hash_value(const String& str);
hash_t hash_value(const WString& wstr);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator>
hash_t hash_value(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return hash_string(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const String& lhs, const String& rhs);
int Compare(const WString& lhs, const WString& rhs);
//----------------------------------------------------------------------------
int CompareI(const String& lhs, const String& rhs);
int CompareI(const WString& lhs, const WString& rhs);
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
template <typename _Char, typename _Traits, typename _Allocator>
String ToString(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return ToString(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator>
WString ToWString(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return ToWString(str.c_str(), str.size());
}
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
