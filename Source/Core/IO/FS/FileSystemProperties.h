#pragma once

#include "Core/Core.h"

#include "Core/IO/String_fwd.h"
#include "Core/IO/StringView.h"
#include "Core/HAL/PlatformFile.h"

namespace Core {
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
    AltSeparator = FPlatformFile::PathSeparatorAlt
};
//----------------------------------------------------------------------------
class TTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char_type ch) const { return FPlatformFile::IsAllowedChar(ch); }
};
//----------------------------------------------------------------------------
using FString = TBasicString<char_type>;
using FStringView = TBasicStringView<char_type>;
//----------------------------------------------------------------------------
inline FileSystem::FStringView Separators() { return MakeStringView(L"/\\"); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace FileSystem
} //!namespace Core
