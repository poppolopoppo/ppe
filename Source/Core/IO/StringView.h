#pragma once

#include "Core/Core.h"

#include "Core/Container/Hash.h"
#include "Core/Memory/MemoryView.h"

#include <cctype>
#include <cwctype>
#include <locale>
#include <iosfwd>
#include <string>
#include <string.h>
#include <wchar.h>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ECase : bool {
    Sensitive   = true,
    Insensitive = false,
};
//----------------------------------------------------------------------------
inline char ToLower(char ch) { return ((ch >= 'A') && (ch <= 'Z')) ? 'a' + (ch - 'A') : ch; }
inline char ToUpper(char ch) { return ((ch >= 'a') && (ch <= 'z')) ? 'A' + (ch - 'a') : ch; }
//----------------------------------------------------------------------------
inline wchar_t ToLower(wchar_t wch) { return std::towlower(wch); }
inline wchar_t ToUpper(wchar_t wch) { return std::towupper(wch); }
//----------------------------------------------------------------------------
template <typename _Char> void InplaceToLower(_Char& ch) { ch = ToLower(ch); }
template <typename _Char> void InplaceToUpper(_Char& ch) { ch = ToUpper(ch); }
//----------------------------------------------------------------------------
CORE_API void ToLower(const TMemoryView<char>& dst, const TMemoryView<const char>& src);
CORE_API void ToLower(const TMemoryView<wchar_t>& dst, const TMemoryView<const wchar_t>& src);
//----------------------------------------------------------------------------
CORE_API void ToUpper(const TMemoryView<char>& dst, const TMemoryView<const char>& src);
CORE_API void ToUpper(const TMemoryView<wchar_t>& dst, const TMemoryView<const wchar_t>& src);
//----------------------------------------------------------------------------
template <typename _Char> void InplaceToLower(const TMemoryView<_Char>& str) { ToLower(str, str); }
template <typename _Char> void InplaceToUpper(const TMemoryView<_Char>& str) { ToUpper(str, str); }
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

    TBasicStringView() = default;

    TBasicStringView(const TBasicStringView& other) = default;
    TBasicStringView& operator =(const TBasicStringView& other) = default;

    TBasicStringView(TBasicStringView&& rvalue) = default;
    TBasicStringView& operator =(TBasicStringView&& rvalue) = default;

    TBasicStringView(const parent_type& other) : parent_type(other) {}
    TBasicStringView& operator =(const parent_type& other) { parent_type::operator =(other); return *this; }

    TBasicStringView(parent_type&& rvalue) : parent_type(std::move(rvalue)) {}
    TBasicStringView& operator =(parent_type&& rvalue) { parent_type::operator =(std::move(rvalue)); return *this; }

    TBasicStringView(const TMemoryView<_Char>& other) : parent_type(other) {}
    TBasicStringView& operator =(const TMemoryView<_Char>& other) { parent_type::operator =(other); return *this; }

    TBasicStringView(std::initializer_list<value_type> list) : parent_type(list) {}
    TBasicStringView(const iterator& first, const iterator& last) : parent_type(first, last) {}
    TBasicStringView(pointer storage, size_type size) : parent_type(storage, size) {}

    const _Char* c_str() const { return parent_type::data(); }

    template <size_t _Dim>
    TBasicStringView(const _Char (&staticChars)[_Dim])
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

    const parent_type& MakeView() const { return *this; }

    auto ToLower() const {
        _Char(*transform)(_Char) = &Core::ToLower;
        return parent_type::Map(transform);
    }

    auto ToUpper() const {
        _Char(*transform)(_Char) = &Core::ToUpper;
        return parent_type::Map(transform);
    }

    friend bool operator ==(const TBasicStringView& lhs, const TBasicStringView& rhs) { return Equals(lhs, rhs); }
    friend bool operator !=(const TBasicStringView& lhs, const TBasicStringView& rhs) { return not operator ==(lhs, rhs); }

    friend bool operator < (const TBasicStringView& lhs, const TBasicStringView& rhs) { return Compare(lhs, rhs); }
    friend bool operator >=(const TBasicStringView& lhs, const TBasicStringView& rhs) { return not operator < (lhs, rhs); }
};
//----------------------------------------------------------------------------
typedef TBasicStringView<char>      FStringView;
typedef TBasicStringView<wchar_t>   FWStringView;
//----------------------------------------------------------------------------
CORE_ASSUME_TYPE_AS_POD(FStringView)
CORE_ASSUME_TYPE_AS_POD(FWStringView)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Allocator >
FORCE_INLINE TBasicStringView<_Char> MakeStringView(const std::basic_string<_Char, _Traits, _Allocator>& str) {
    return TBasicStringView<_Char>(str.c_str(), str.size());
}
//----------------------------------------------------------------------------
template <size_t _Dim>
FORCE_INLINE FStringView MakeStringView(const char(&cstr)[_Dim]) {
    return FStringView(cstr);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
FORCE_INLINE FWStringView MakeStringView(const wchar_t(&cstr)[_Dim]) {
    return FWStringView(cstr);
}
//----------------------------------------------------------------------------
FORCE_INLINE FStringView MakeStringView(const TMemoryView<const char>& view) {
    return FStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
FORCE_INLINE FWStringView MakeStringView(const TMemoryView<const wchar_t>& view) {
    return FWStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
FORCE_INLINE FStringView MakeStringView(const TMemoryView<char>& view) {
    return FStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
FORCE_INLINE FWStringView MakeStringView(const TMemoryView<wchar_t>& view) {
    return FWStringView(view.data(), view.size());
}
//----------------------------------------------------------------------------
FORCE_INLINE const FStringView& MakeStringView(const FStringView& view) {
    return view;
}
//----------------------------------------------------------------------------
FORCE_INLINE const FWStringView& MakeStringView(const FWStringView& view) {
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
FORCE_INLINE TBasicStringView<_Char> MakeStringView(const _Char* cstr, Meta::FForceInit ) {
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
FStringView::iterator StrChr(const FStringView& str, char ch);
FStringView::reverse_iterator StrRChr(const FStringView& str, char ch);
//----------------------------------------------------------------------------
FWStringView::iterator StrChr(const FWStringView& wstr, wchar_t wch);
FWStringView::reverse_iterator StrRChr(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
FStringView::iterator StrStr(const FStringView& str, const FStringView& firstOccurence);
FWStringView::iterator StrStr(const FWStringView& wstr, const FWStringView& firstOccurence);
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
FORCE_INLINE bool IsIdentifier(char ch) { return (IsAlnum(ch) || ch == '_' || ch == '.'); }
FORCE_INLINE bool IsIdentifier(wchar_t wch) { return (IsAlnum(wch) || wch == L'_' || wch == L'.'); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsPrint(char ch) { return 0 != std::isprint((int)((unsigned char)ch)); }
FORCE_INLINE bool IsPrint(wchar_t wch) { return 0 != std::iswprint(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsPunct(char ch) { return 0 != std::ispunct((int)((unsigned char)ch)); }
FORCE_INLINE bool IsPunct(wchar_t wch) { return 0 != std::iswpunct(wch); }
//----------------------------------------------------------------------------
FORCE_INLINE bool IsSpace(char ch) { return ( ch == ' ' || ch == '\f' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' ); }
FORCE_INLINE bool IsSpace(wchar_t wch) { return 0 != std::iswdigit(wch); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool IsAlnum(const FStringView& str);
bool IsAlnum(const FWStringView& wstr);
//----------------------------------------------------------------------------
bool IsAlpha(const FStringView& str);
bool IsAlpha(const FWStringView& wstr);
//----------------------------------------------------------------------------
bool IsDigit(const FStringView& str);
bool IsDigit(const FWStringView& wstr);
//----------------------------------------------------------------------------
bool IsXDigit(const FStringView& str);
bool IsXDigit(const FWStringView& wstr);
//----------------------------------------------------------------------------
bool IsIdentifier(const FStringView& str);
bool IsIdentifier(const FWStringView& wstr);
//----------------------------------------------------------------------------
bool IsPrint(const FStringView& str);
bool IsPrint(const FWStringView& wstr);
//----------------------------------------------------------------------------
bool IsSpace(const FStringView& str);
bool IsSpace(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView EatAlnums(FStringView& str);
FWStringView EatAlnums(FWStringView& wstr);
//----------------------------------------------------------------------------
FStringView EatAlphas(FStringView& str);
FWStringView EatAlphas(FWStringView& wstr);
//----------------------------------------------------------------------------
FStringView EatDigits(FStringView& str);
FWStringView EatDigits(FWStringView& wstr);
//----------------------------------------------------------------------------
FStringView EatXDigits(FStringView& str);
FWStringView EatXDigits(FWStringView& wstr);
//----------------------------------------------------------------------------
FStringView EatPrints(FStringView& str);
FWStringView EatPrints(FWStringView& wstr);
//----------------------------------------------------------------------------
FStringView EatSpaces(FStringView& str);
FWStringView EatSpaces(FWStringView& wstr);
//----------------------------------------------------------------------------
FStringView Chomp(const FStringView& str);
FWStringView Chomp(const FWStringView& wstr);
//----------------------------------------------------------------------------
FStringView Strip(const FStringView& str);
FWStringView Strip(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
int Compare(const FStringView& lhs, const FStringView& rhs);
int Compare(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
int CompareI(const FStringView& lhs, const FStringView& rhs);
int CompareI(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool EqualsN(const char* lhs, const char* rhs, size_t len);
bool EqualsNI(const char* lhs, const char* rhs, size_t len);
//----------------------------------------------------------------------------
bool EqualsN(const wchar_t* lhs, const wchar_t* rhs, size_t len);
bool EqualsNI(const wchar_t* lhs, const wchar_t* rhs, size_t len);
//----------------------------------------------------------------------------
bool Equals(const FStringView& lhs, const FStringView& rhs);
bool Equals(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
bool EqualsI(const FStringView& lhs, const FStringView& rhs);
bool EqualsI(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
inline bool EqualsI(char lhs, char rhs) { return (ToUpper(lhs) == ToUpper(rhs)); }
inline bool EqualsI(wchar_t lhs, wchar_t rhs) { return (ToUpper(lhs) == ToUpper(rhs)); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool StartsWith(const FStringView& str, const FStringView& prefix);
bool StartsWith(const FWStringView& wstr, const FWStringView& wprefix);
//----------------------------------------------------------------------------
bool StartsWithI(const FStringView& str, const FStringView& prefix);
bool StartsWithI(const FWStringView& wstr, const FWStringView& wprefix);
//----------------------------------------------------------------------------
bool EndsWith(const FStringView& str, const FStringView& suffix);
bool EndsWith(const FWStringView& wstr, const FWStringView& wsuffix);
//----------------------------------------------------------------------------
bool EndsWithI(const FStringView& str, const FStringView& suffix);
bool EndsWithI(const FWStringView& wstr, const FWStringView& wsuffix);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Split(FStringView& str, char separator, FStringView& slice);
bool Split(FWStringView& wstr, wchar_t separator, FWStringView& slice);
//----------------------------------------------------------------------------
bool Split(FStringView& str, const FStringView& separators, FStringView& slice);
bool Split(FWStringView& wstr, const FWStringView& separators, FWStringView& slice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const FStringView& str, size_t base);
bool Atoi64(i64* dst, const FStringView& str, size_t base);
//----------------------------------------------------------------------------
bool Atoi32(i32* dst, const FWStringView& wstr, size_t base);
bool Atoi64(i64* dst, const FWStringView& wstr, size_t base);
//----------------------------------------------------------------------------
template <typename _Char>
bool Atoi(intptr_t* dst, const TBasicStringView<_Char>& str, size_t base) {
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
bool Atoi(size_t* dst, const TBasicStringView<_Char>& str, size_t base) {
    return Atoi((intptr_t*)dst, str, base);
}
//----------------------------------------------------------------------------
bool Atof(float* dst, const FStringView& str);
bool Atod(double* dst, const FStringView& str);
//----------------------------------------------------------------------------
bool Atof(float* dst, const FWStringView& wstr);
bool Atod(double* dst, const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool WildMatch(const FStringView& pattern, const FStringView& str);
bool WildMatch(const FWStringView& pattern, const FWStringView& wstr);
//----------------------------------------------------------------------------
bool WildMatchI(const FStringView& pattern, const FStringView& str);
bool WildMatchI(const FWStringView& pattern, const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t EditDistance(const FStringView& lhs, const FStringView& rhs);
size_t EditDistance(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
size_t EditDistanceI(const FStringView& lhs, const FStringView& rhs);
size_t EditDistanceI(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t Copy(const TMemoryView<char>& dst, const FStringView& src);
size_t Copy(const TMemoryView<wchar_t>& dst, const FWStringView& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
hash_t hash_string(const FStringView& str);
hash_t hash_string(const FWStringView& wstr);
//----------------------------------------------------------------------------
hash_t hash_stringI(const FStringView& str);
hash_t hash_stringI(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TCharEqualTo {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return lhs == rhs; }
};
template <typename _Char>
struct TCharEqualTo<_Char, ECase::Insensitive> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return ToLower(lhs) == ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TCharLess {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return lhs < rhs; }
};
template <typename _Char>
struct TCharLess<_Char, ECase::Insensitive> {
    bool operator ()(const _Char& lhs, const _Char& rhs) const { return ToLower(lhs) < ToLower(rhs); }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TCharCase {
    _Char operator ()(const _Char& ch) const { return ch; }
};
template <typename _Char>
struct TCharCase<_Char, ECase::Insensitive> {
    _Char operator ()(const _Char& ch) const { return ToLower(ch); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewEqualTo {
    bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const {
        return Equals(lhs, rhs);
    }
};
template <typename _Char>
struct TStringViewEqualTo<_Char, ECase::Insensitive> {
    bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const {
        return EqualsI(lhs, rhs);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewLess {
    bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const {
        return (Compare(lhs, rhs) < 0);
    }
};
template <typename _Char>
struct TStringViewLess<_Char, ECase::Insensitive> {
    bool operator ()(const TBasicStringView<_Char>& lhs, const TBasicStringView<_Char>& rhs) const {
        return (CompareI(lhs, rhs) < 0);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewHasher {
    hash_t operator ()(const TBasicStringView<_Char>& str) const {
        return hash_string(str);
    }
};
template <typename _Char>
struct TStringViewHasher<_Char, ECase::Insensitive> {
    hash_t operator ()(const TBasicStringView<_Char>& str) const {
        return hash_stringI(str);
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
using TBasicStringViewHashMemoizer = THashMemoizer<
    TBasicStringView<_Char>,
    TStringViewHasher<_Char, _Sensitive>,
    TStringViewEqualTo<_Char, _Sensitive>
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicConstChar {
    const _Char* Data = nullptr;

    TBasicConstChar(const _Char* data) : Data(data) {}
    TBasicConstChar(const TBasicStringView<_Char>& str) : Data(str.c_str()) {}

    const _Char* c_str() const { return Data; }
    operator const _Char* () const { return Data; }

    TBasicStringView<_Char> MakeView() const { return TBasicStringView<_Char>(Data, Length(Data)); }
};
//----------------------------------------------------------------------------
using FConstChar = TBasicConstChar<char>;
using FConstWChar = TBasicConstChar<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TConstCharEqualTo : TStringViewEqualTo<_Char, _Sensitive> {
    bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicConstChar<_Char>& rhs) const {
        return (lhs.Data == rhs.Data || TStringViewEqualTo<_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs.MakeView()));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TConstCharLess : TStringViewLess<_Char, _Sensitive> {
    bool operator ()(const TBasicConstChar<_Char>& lhs, const TBasicConstChar<_Char>& rhs) const {
        return (lhs.Data == rhs.Data ? 0 : TStringViewLess<_Char, _Sensitive>::operator ()(lhs.MakeView(), rhs.MakeView()));
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TConstCharHasher : TStringViewHasher<_Char, _Sensitive> {
    hash_t operator ()(const TBasicConstChar<_Char>& cstr) const {
        return TStringViewHasher<_Char, _Sensitive>::operator ()(cstr.MakeView());
    }
};
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
using TBasicConstCharHashMemoizer = THashMemoizer<
    TBasicConstChar<_Char>,
    TConstCharHasher<_Char, _Sensitive>,
    TConstCharEqualTo<_Char, _Sensitive>
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EEscape {
    Octal = 0,
    Hexadecimal,
    Unicode,
};
CORE_API void Escape(std::basic_ostream<char>& oss, const FStringView& str, EEscape escape);
CORE_API void Escape(std::basic_ostream<wchar_t>& oss, const FWStringView& wstr, EEscape escape);
//----------------------------------------------------------------------------
CORE_API std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const FStringView& slice);
CORE_API std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const FWStringView& wslice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
