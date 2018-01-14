#pragma once

#include "Core/Core.h"

namespace Core {
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
class TBasicString;
//----------------------------------------------------------------------------
using FString = TBasicString<char>;
using FWString = TBasicString<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicStringBuilder;
//----------------------------------------------------------------------------
using FStringBuilder = TBasicStringBuilder<char>;
using FWStringBuilder = TBasicStringBuilder<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
