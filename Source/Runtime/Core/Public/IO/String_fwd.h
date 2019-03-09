#pragma once

#include "Core_fwd.h"

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
class TBasicStringBuilder;
//----------------------------------------------------------------------------
using FStringBuilder = TBasicStringBuilder<char>;
using FWStringBuilder = TBasicStringBuilder<wchar_t>;
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
} //!namespace PPE
