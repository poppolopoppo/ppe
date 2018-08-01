#pragma once

#include "Core.h"

#include "Container/Hash.h"
#include "IO/String_fwd.h"
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
inline char ToLower(char ch) { return ((ch >= 'A') && (ch <= 'Z')) ? 'a' + (ch - 'A') : ch; }
inline char ToUpper(char ch) { return ((ch >= 'a') && (ch <= 'z')) ? 'A' + (ch - 'a') : ch; }
//----------------------------------------------------------------------------
inline wchar_t ToLower(wchar_t wch) { return std::towlower(wch); }
inline wchar_t ToUpper(wchar_t wch) { return std::towupper(wch); }
//----------------------------------------------------------------------------
template <typename _Char> void InplaceToLower(_Char& ch) { ch = ToLower(ch); }
template <typename _Char> void InplaceToUpper(_Char& ch) { ch = ToUpper(ch); }
//----------------------------------------------------------------------------
PPE_API void ToLower(const TMemoryView<char>& dst, const TMemoryView<const char>& src);
PPE_API void ToLower(const TMemoryView<wchar_t>& dst, const TMemoryView<const wchar_t>& src);
//----------------------------------------------------------------------------
PPE_API void ToUpper(const TMemoryView<char>& dst, const TMemoryView<const char>& src);
PPE_API void ToUpper(const TMemoryView<wchar_t>& dst, const TMemoryView<const wchar_t>& src);
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

    TBasicStringView(const TMemoryView<_Char>& other) : parent_type(other) {}
    TBasicStringView& operator =(const TMemoryView<_Char>& other) { parent_type::operator =(other); return *this; }

    TBasicStringView(parent_type&& rvalue) : parent_type(std::move(rvalue)) {}
    TBasicStringView& operator =(parent_type&& rvalue) { parent_type::operator =(std::move(rvalue)); return *this; }

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
        Assert(_Dim > parent_type::size());
        parent_type::CopyTo(dst);
        dst[parent_type::size()] = _Char(0);
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
PPE_ASSUME_TYPE_AS_POD(FStringView)
PPE_ASSUME_TYPE_AS_POD(FWStringView)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
FORCE_INLINE TBasicStringView<_Char> MakeStringView(const TBasicString<_Char>& str) {
    return TBasicStringView<_Char>(str.data(), str.size());
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
PPE_API FStringView::iterator StrChr(const FStringView& str, char ch);
PPE_API FStringView::reverse_iterator StrRChr(const FStringView& str, char ch);
//----------------------------------------------------------------------------
PPE_API FWStringView::iterator StrChr(const FWStringView& wstr, wchar_t wch);
PPE_API FWStringView::reverse_iterator StrRChr(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_API FStringView::iterator StrStr(const FStringView& str, const FStringView& firstOccurence);
PPE_API FWStringView::iterator StrStr(const FWStringView& wstr, const FWStringView& firstOccurence);
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
PPE_API bool IsAlnum(const FStringView& str);
PPE_API bool IsAlnum(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API bool IsAlpha(const FStringView& str);
PPE_API bool IsAlpha(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API bool IsDigit(const FStringView& str);
PPE_API bool IsDigit(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API bool IsXDigit(const FStringView& str);
PPE_API bool IsXDigit(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API bool IsIdentifier(const FStringView& str);
PPE_API bool IsIdentifier(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API bool IsPrint(const FStringView& str);
PPE_API bool IsPrint(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API bool IsSpace(const FStringView& str);
PPE_API bool IsSpace(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API FStringView EatAlnums(FStringView& str);
PPE_API FWStringView EatAlnums(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API FStringView EatAlphas(FStringView& str);
PPE_API FWStringView EatAlphas(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API FStringView EatDigits(FStringView& str);
PPE_API FWStringView EatDigits(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API FStringView EatXDigits(FStringView& str);
PPE_API FWStringView EatXDigits(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API FStringView EatPrints(FStringView& str);
PPE_API FWStringView EatPrints(FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API FStringView EatSpaces(FStringView& str);
PPE_API FWStringView EatSpaces(FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API FStringView TrimStart(const FStringView& str, char ch);
PPE_API FWStringView TrimStart(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_API FStringView TrimStart(const FStringView& str, const FStringView& chars);
PPE_API FWStringView TrimStart(const FWStringView& wstr, const FWStringView& wchars);
//----------------------------------------------------------------------------
PPE_API FStringView TrimEnd(const FStringView& str, char ch);
PPE_API FWStringView TrimEnd(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_API FStringView TrimEnd(const FStringView& str, const FStringView& chars);
PPE_API FWStringView TrimEnd(const FWStringView& wstr, const FWStringView& wchars);
//----------------------------------------------------------------------------
PPE_API FStringView Trim(const FStringView& str, char ch);
PPE_API FWStringView Trim(const FWStringView& wstr, wchar_t wch);
//----------------------------------------------------------------------------
PPE_API FStringView Trim(const FStringView& str, const FStringView& chars);
PPE_API FWStringView Trim(const FWStringView& wstr, const FWStringView& wchars);
//----------------------------------------------------------------------------
PPE_API FStringView Chomp(const FStringView& str);
PPE_API FWStringView Chomp(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API FStringView Strip(const FStringView& str);
PPE_API FWStringView Strip(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API int Compare(const FStringView& lhs, const FStringView& rhs);
PPE_API int Compare(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
PPE_API int CompareI(const FStringView& lhs, const FStringView& rhs);
PPE_API int CompareI(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API bool EqualsN(const char* lhs, const char* rhs, size_t len);
PPE_API bool EqualsNI(const char* lhs, const char* rhs, size_t len);
//----------------------------------------------------------------------------
PPE_API bool EqualsN(const wchar_t* lhs, const wchar_t* rhs, size_t len);
PPE_API bool EqualsNI(const wchar_t* lhs, const wchar_t* rhs, size_t len);
//----------------------------------------------------------------------------
PPE_API bool Equals(const FStringView& lhs, const FStringView& rhs);
PPE_API bool Equals(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
PPE_API bool EqualsI(const FStringView& lhs, const FStringView& rhs);
PPE_API bool EqualsI(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
inline bool EqualsI(char lhs, char rhs) { return (ToUpper(lhs) == ToUpper(rhs)); }
inline bool EqualsI(wchar_t lhs, wchar_t rhs) { return (ToUpper(lhs) == ToUpper(rhs)); }
//----------------------------------------------------------------------------
inline bool Equals(char lhs, char rhs, ECase cmp) { return (ECase::Insensitive == cmp ? ToUpper(lhs) == ToUpper(rhs) : lhs == rhs); }
inline bool Equals(wchar_t lhs, wchar_t rhs, ECase cmp) { return (ECase::Insensitive == cmp ? ToUpper(lhs) == ToUpper(rhs) : lhs == rhs); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API bool StartsWith(const FStringView& str, const FStringView& prefix);
PPE_API bool StartsWith(const FWStringView& wstr, const FWStringView& wprefix);
//----------------------------------------------------------------------------
PPE_API bool StartsWithI(const FStringView& str, const FStringView& prefix);
PPE_API bool StartsWithI(const FWStringView& wstr, const FWStringView& wprefix);
//----------------------------------------------------------------------------
PPE_API bool EndsWith(const FStringView& str, const FStringView& suffix);
PPE_API bool EndsWith(const FWStringView& wstr, const FWStringView& wsuffix);
//----------------------------------------------------------------------------
PPE_API bool EndsWithI(const FStringView& str, const FStringView& suffix);
PPE_API bool EndsWithI(const FWStringView& wstr, const FWStringView& wsuffix);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API bool Split(FStringView& str, char separator, FStringView& slice);
PPE_API bool Split(FWStringView& wstr, wchar_t separator, FWStringView& slice);
//----------------------------------------------------------------------------
PPE_API bool Split(FStringView& str, const FStringView& separators, FStringView& slice);
PPE_API bool Split(FWStringView& wstr, const FWStringView& separators, FWStringView& slice);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API bool Atoi(i32* dst, const FStringView& str, size_t base);
PPE_API bool Atoi(u32* dst, const FStringView& str, size_t base);
PPE_API bool Atoi(i64* dst, const FStringView& str, size_t base);
PPE_API bool Atoi(u64* dst, const FStringView& str, size_t base);
//----------------------------------------------------------------------------
PPE_API bool Atoi(i32* dst, const FWStringView& wstr, size_t base);
PPE_API bool Atoi(u32* dst, const FWStringView& wstr, size_t base);
PPE_API bool Atoi(i64* dst, const FWStringView& wstr, size_t base);
PPE_API bool Atoi(u64* dst, const FWStringView& wstr, size_t base);
//----------------------------------------------------------------------------
PPE_API bool Atof(float* dst, const FStringView& str);
PPE_API bool Atod(double* dst, const FStringView& str);
//----------------------------------------------------------------------------
PPE_API bool Atof(float* dst, const FWStringView& wstr);
PPE_API bool Atod(double* dst, const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API bool WildMatch(const FStringView& pattern, const FStringView& str);
PPE_API bool WildMatch(const FWStringView& pattern, const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API bool WildMatchI(const FStringView& pattern, const FStringView& str);
PPE_API bool WildMatchI(const FWStringView& pattern, const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API size_t EditDistance(const FStringView& lhs, const FStringView& rhs);
PPE_API size_t EditDistance(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
PPE_API size_t EditDistanceI(const FStringView& lhs, const FStringView& rhs);
PPE_API size_t EditDistanceI(const FWStringView& lhs, const FWStringView& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API size_t Copy(const TMemoryView<char>& dst, const FStringView& src);
PPE_API size_t Copy(const TMemoryView<wchar_t>& dst, const FWStringView& src);
//----------------------------------------------------------------------------
PPE_API const char* NullTerminated(const TMemoryView<char>& dst, const FStringView& src);
PPE_API const wchar_t* NullTerminated(const TMemoryView<wchar_t>& dst, const FWStringView& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_API hash_t hash_string(const FStringView& str);
PPE_API hash_t hash_string(const FWStringView& wstr);
//----------------------------------------------------------------------------
PPE_API hash_t hash_stringI(const FStringView& str);
PPE_API hash_t hash_stringI(const FWStringView& wstr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EEscape {
    Octal = 0,
    Hexadecimal,
    Unicode,
};
//----------------------------------------------------------------------------
PPE_API void Escape(FTextWriter& oss, const FStringView& str, EEscape escape);
PPE_API void Escape(FWTextWriter& oss, const FWStringView& str, EEscape escape);
//----------------------------------------------------------------------------
PPE_API void Escape(FTextWriter& oss, const FStringView& wstr, EEscape escape);
PPE_API void Escape(FWTextWriter& oss, const FWStringView& str, EEscape escape);
//----------------------------------------------------------------------------
PPE_API FTextWriter& operator <<(FTextWriter& oss, const FWStringView& wslice);
PPE_API FWTextWriter& operator <<(FWTextWriter& oss, const FStringView& slice);
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
} //!namespace PPE
