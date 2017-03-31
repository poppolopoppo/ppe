#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/Stream.h"
#include "Core/IO/StringView.h"
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
class TBasicString : public std::basic_string<_Char, _Traits, _Allocator> {
public:
    typedef std::basic_string<_Char, _Traits, _Allocator> parent_type;

    using parent_type::parent_type;
    using parent_type::operator=;

    TBasicString() = default;

    TBasicString(const TBasicString& other) = default;
    TBasicString& operator =(const TBasicString& other) = default;

    TBasicString(TBasicString&& rvalue) = default;
    TBasicString& operator =(TBasicString&& rvalue) = default;

    TBasicString(const parent_type& other) : parent_type(other) {}
    TBasicString& operator =(const parent_type& other) { parent_type::operator =(other); return *this; }

    TBasicString(parent_type&& rvalue) : parent_type(std::move(rvalue)) {}
    TBasicString& operator =(parent_type&& rvalue) { parent_type::operator =(std::move(rvalue)); return *this; }

    TBasicStringView<_Char> MakeView() const { return TBasicStringView<_Char>(c_str(), size()); }
    operator TBasicStringView<_Char> () const { return MakeView(); }
};
//----------------------------------------------------------------------------
typedef TBasicString<char>      FString;
typedef TBasicString<wchar_t>   FWString;
//----------------------------------------------------------------------------
hash_t hash_value(const FString& str);
hash_t hash_value(const FWString& wstr);
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator>
hash_t hash_value(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return hash_string(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const FString& lhs, const FString& rhs);
int Compare(const FWString& lhs, const FWString& rhs);
//----------------------------------------------------------------------------
int CompareI(const FString& lhs, const FString& rhs);
int CompareI(const FWString& lhs, const FWString& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr, size_t length);
size_t ToCStr(char *dst, size_t capacity, const wchar_t *wcstr);
size_t ToCStr(char *dst, size_t capacity, const FWString& wstr);
//----------------------------------------------------------------------------
size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr, size_t length);
size_t ToWCStr(wchar_t *dst, size_t capacity, const char *cstr);
size_t ToWCStr(wchar_t *dst, size_t capacity, const FString& str);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FString ToString(const wchar_t *wcstr, size_t length);
FString ToString(const wchar_t *wcstr);
FString ToString(const FWString& wstr);
inline const FString& ToString(const FString& str) { return str; }
inline FString ToString(const TMemoryView<const char>& strview) { return FString(strview.Pointer(), strview.size()); }
inline FString ToString(const TMemoryView<const wchar_t>& strview) { return ToString(strview.Pointer(), strview.size()); }
//----------------------------------------------------------------------------
FWString ToWString(const char *cstr, size_t length);
FWString ToWString(const char *cstr);
FWString ToWString(const FString& str);
inline const FWString& ToWString(const FWString& wstr) { return wstr; }
inline FWString ToWString(const TMemoryView<const wchar_t>& strview) { return FWString(strview.Pointer(), strview.size()); }
inline FWString ToWString(const TMemoryView<const char>& strview) { return ToWString(strview.Pointer(), strview.size()); }
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator>
FString ToString(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return ToString(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator>
FWString ToWString(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return ToWString(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <typename _Char>
FString ToString(const TBasicStringView<_Char>& str) {
    return ToString(str.MakeView());
}
//----------------------------------------------------------------------------
template <typename _Char>
FWString ToWString(const TBasicStringView<_Char>& str) {
    return ToWString(str.MakeView());
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
    const TBasicString<_Char2, _Traits2, _Allocator>& str) {
    return oss << str.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringEqualTo : TStringViewEqualTo <_Char, _Sensitive> {
    bool operator ()(const TBasicString<_Char>& lhs, const TBasicString<_Char>& rhs) const {
        return TStringViewEqualTo <_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs.MakeView());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringLess : TStringViewLess <_Char, _Sensitive> {
    bool operator ()(const TBasicString<_Char>& lhs, const TBasicString<_Char>& rhs) const {
        return TStringViewLess <_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs.MakeView());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringHasher : TStringViewHasher<_Char, _Sensitive> {
    size_t operator ()(const TBasicString<_Char>& str) const {
        return TStringViewHasher<_Char, _Sensitive>::operator ()(str.MakeView());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
using TBasicStringHashMemoizer = THashMemoizer<
    TBasicString<_Char>,
    TStringHasher<_Char, _Sensitive>,
    TStringEqualTo<_Char, _Sensitive>
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/String-inl.h"
