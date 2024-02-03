#pragma once

#include "Core_fwd.h"
#include "Meta/Assert.h"

#define STRING_LITERAL(_CHAR, _ANSI) \
    ::PPE::details::MakeStringLiteral( (_CHAR{}), (_ANSI), WIDESTRING(_ANSI) )

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Stores a null-terminated string with storage ownership and small-string-buffer-optimization (fixed allocator)
//----------------------------------------------------------------------------
enum class ECase : bool {
    Sensitive   = true,
    Insensitive = false,
};
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicString;
//----------------------------------------------------------------------------
using FString = TBasicString<char>;
using FWString = TBasicString<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringEqualTo;
template <typename _Char, ECase _Sensitive>
struct TStringLess;
template <typename _Char, ECase _Sensitive>
struct TStringHasher;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// String view points to a range of character, not guaranteed to be null-terminated but very easy to use for parsing
//----------------------------------------------------------------------------
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringView;
//----------------------------------------------------------------------------
using FStringView = TBasicStringView<char>;
using FWStringView = TBasicStringView<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> MakeCStringView(const _Char* cstr) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> MakeStringView(const TBasicString<_Char>& str) NOEXCEPT;
template <size_t _Dim>
CONSTEXPR FStringView MakeStringView(const char(&cstr)[_Dim]) NOEXCEPT;
template <size_t _Dim>
CONSTEXPR FWStringView MakeStringView(const wchar_t(&cstr)[_Dim]) NOEXCEPT;
CONSTEXPR FStringView MakeStringView(const TMemoryView<const char>& view) NOEXCEPT;
CONSTEXPR FWStringView MakeStringView(const TMemoryView<const wchar_t>& view) NOEXCEPT;
CONSTEXPR FStringView MakeStringView(const TMemoryView<char>& view) NOEXCEPT;
CONSTEXPR FWStringView MakeStringView(const TMemoryView<wchar_t>& view) NOEXCEPT;
CONSTEXPR const FStringView& MakeStringView(const FStringView& view) NOEXCEPT;
CONSTEXPR const FWStringView& MakeStringView(const FWStringView& view) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TStringViewEqualTo;
template <typename _Char, ECase _Sensitive>
struct TStringViewLess;
template <typename _Char, ECase _Sensitive>
struct TStringViewHasher;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Stores a pointer to a null-terminated string, without allocation ownership (smaller view)
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicConstChar;
//----------------------------------------------------------------------------
using FConstChar = TBasicConstChar<char>;
using FConstWChar = TBasicConstChar<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TConstCharEqualTo;
template <typename _Char, ECase _Sensitive>
struct TConstCharLess;
template <typename _Char, ECase _Sensitive>
struct TConstCharHasher;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Wraps a an array of characters with a predefined size, used for transient charset conversions
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity>
struct TBasicStaticString;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use a string builder if you can't predict the size of a string
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
class TGenericStringBuilder;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringBuilder;
//----------------------------------------------------------------------------
using FStringBuilder = TBasicStringBuilder<char>;
using FWStringBuilder = TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Parse strings to bool, int, float or custom types if overriding operator >>
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicStringConversion;
//----------------------------------------------------------------------------
using FStringConversion = TBasicStringConversion<char>;
using FWStringConversion = TBasicStringConversion<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Helpers for string formatting with type erasure
//----------------------------------------------------------------------------
namespace details {
template <typename _Char>
struct TBasicFormatFunctor_;
} //!details
//----------------------------------------------------------------------------
template <typename _Char>
using TBasicFormatArg = details::TBasicFormatFunctor_<_Char>;
using FFormatArg = TBasicFormatArg<char>;
using FWFormatArg = TBasicFormatArg<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
using TBasicFormatArgList = TMemoryView< const TBasicFormatArg<_Char> >;
using FFormatArgList = TBasicFormatArgList<char>;
using FWFormatArgList = TBasicFormatArgList<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Points to a null-terminated literal string with static lifetime
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringLiteral;
using FStringLiteral = TBasicStringLiteral<char>;
using FWStringLiteral = TBasicStringLiteral<wchar_t>;
//----------------------------------------------------------------------------
// Helpers to make literal string  ANSI/WIDE agnostic:
//----------------------------------------------------------------------------
namespace details {
template <typename _Ansi, typename _Wide>
CONSTEXPR _Ansi MakeStringLiteral(char, _Ansi&& ansi, _Wide&& ) { return ansi; }
template <typename _Ansi, typename _Wide>
CONSTEXPR _Wide MakeStringLiteral(wchar_t, _Ansi&& , _Wide&& wide) { return wide; }
} //!details
template <typename _Char, size_t _Len>
CONSTEXPR auto MakeStringLiteral(const _Char (&literalString)[_Len]) {
    return TBasicStringLiteral<_Char>{ literalString };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Stores a string with a custom allocator, content can't be modified once created
//----------------------------------------------------------------------------
template <typename _Char, typename _Allocator>
class TBasicText;
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive>
struct TTextEqualTo;
template <typename _Char, ECase _Sensitive>
struct TTextLess;
template <typename _Char, ECase _Sensitive>
struct TTextHasher;
//----------------------------------------------------------------------------
// Helper to merge identical strings and avoid allocating them more than once
//----------------------------------------------------------------------------
template <typename _Char, ECase _Sensitive, typename _Allocator>
class TBasicTextMemoization;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
