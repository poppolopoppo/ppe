#pragma once

#include "Core.h"

#include "Container/Token_fwd.h"
#include "IO/FileSystemProperties.h"
#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FFileSystemTokenTag;
//----------------------------------------------------------------------------
using FFileSystemToken = PPE::TToken<
    FFileSystemTokenTag,
    FileSystem::char_type,
    FileSystem::CaseSensitive,
    FileSystem::TTokenTraits
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
