#pragma once

#include "Core.h"

#include "IO/FileSystemToken_fwd.h"

#include "Container/Token.h"
#include "IO/FileSystemProperties.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFileSystemTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(FileSystem::char_type ch) const { return FPlatformFile::IsAllowedChar(ch); }
};
//----------------------------------------------------------------------------
BASICTOKEN_CLASS_DECL(PPE_CORE_API, FFileSystemToken, FileSystem::char_type, FileSystem::CaseSensitive, FFileSystemTokenTraits);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
