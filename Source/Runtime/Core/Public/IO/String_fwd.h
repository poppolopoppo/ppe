#pragma once

#include "Core_fwd.h"

#define STRING_LITERAL(_CHAR, _ASCII) \
    ::PPE::string_literal< _CHAR >( _ASCII, WIDESTRING(_ASCII) )

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ECase : bool;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicString;
//----------------------------------------------------------------------------
using FString = TBasicString<char>;
using FWString = TBasicString<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TMemoryView;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringView;
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity>
struct TBasicStaticString;
//----------------------------------------------------------------------------
using FStringView = TBasicStringView<char>;
using FWStringView = TBasicStringView<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> MakeCStringView(const _Char* cstr);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringView<_Char> MakeStringView(const TBasicString<_Char>& str);
template <size_t _Dim>
CONSTEXPR FStringView MakeStringView(const char(&cstr)[_Dim]);
template <size_t _Dim>
CONSTEXPR FWStringView MakeStringView(const wchar_t(&cstr)[_Dim]);
CONSTEXPR FStringView MakeStringView(const TMemoryView<const char>& view);
CONSTEXPR FWStringView MakeStringView(const TMemoryView<const wchar_t>& view);
CONSTEXPR FStringView MakeStringView(const TMemoryView<char>& view);
CONSTEXPR FWStringView MakeStringView(const TMemoryView<wchar_t>& view);
CONSTEXPR const FStringView& MakeStringView(const FStringView& view);
CONSTEXPR const FWStringView& MakeStringView(const FWStringView& view);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicConstChar;
//----------------------------------------------------------------------------
using FConstChar = TBasicConstChar<char>;
using FConstWChar = TBasicConstChar<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringBuilder;
//----------------------------------------------------------------------------
using FStringBuilder = TBasicStringBuilder<char>;
using FWStringBuilder = TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicStringConversion;
//----------------------------------------------------------------------------
using FStringConversion = TBasicStringConversion<char>;
using FWStringConversion = TBasicStringConversion<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
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
// Macro to make string literal ASCII/WIDE agnostic:
namespace details {
template <typename _Ascii, typename _Wide>
CONSTEXPR _Ascii select_string_literal_(char, _Ascii ascii, _Wide ) { return ascii; }
template <typename _Ascii, typename _Wide>
CONSTEXPR _Wide select_string_literal_(wchar_t, _Ascii , _Wide wide) { return wide; }
} //!details
template <typename _Char>
CONSTEXPR auto string_literal(const char ascii, const wchar_t wide) {
    return details::select_string_literal_(_Char{}, ascii, wide);
}
template <typename _Char>
CONSTEXPR auto string_literal(const FStringView& ascii, const FWStringView& wide) {
    return details::select_string_literal_(_Char{}, ascii, wide);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
