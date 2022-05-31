#pragma once

#include "Core.h"

#include "IO/String_fwd.h"
#include "IO/StringView.h"
#include "HAL/PlatformFile.h"

namespace PPE {
namespace FileSystem {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef FPlatformFile::char_type char_type;
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(ECase, CaseSensitive, FPlatformFile::IsCaseSensitive ? ECase::Sensitive : ECase::Insensitive);
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, MaxPathLength, FPlatformFile::MaxPathLength);
//----------------------------------------------------------------------------
enum : char_type {
    Separator = FPlatformFile::PathSeparator,
    AltSeparator = FPlatformFile::PathSeparatorAlt,
    NormalizedSeparator = L'/'
};
//----------------------------------------------------------------------------
using FString = TBasicString<char_type>;
using FStringView = TBasicStringView<char_type>;
//----------------------------------------------------------------------------
inline FileSystem::FStringView Separators() { return MakeStringView(L"/\\"); }
//----------------------------------------------------------------------------
PPE_CORE_API void Sanitize(const TMemoryView<char_type>& str) NOEXCEPT;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace FileSystem
} //!namespace PPE
