#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicTextWriter;
//----------------------------------------------------------------------------
using FTextWriter = TBasicTextWriter<char>;
using FWTextWriter = TBasicTextWriter<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicFixedSizeTextWriter;
//----------------------------------------------------------------------------
using FFixedSizeTextWriter = TBasicFixedSizeTextWriter<char>;
using FWFixedSizeTextWriter = TBasicFixedSizeTextWriter<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE