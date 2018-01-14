#pragma once

#include "Core/Core.h"

#include "Core/Container/Token_fwd.h"
#include "Core/IO/FS/FileSystemProperties.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FFileSystemTokenTag;
//----------------------------------------------------------------------------
using FFileSystemToken = Core::TToken<
    FFileSystemTokenTag,
    FileSystem::char_type,
    FileSystem::CaseSensitive,
    FileSystem::TTokenTraits
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
