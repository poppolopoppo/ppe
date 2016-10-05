#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemProperties.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename        _Tag,
    typename        _Char,
    ECase            _Sensitive,
    typename        _TokenTraits,
    typename        _Allocator
>
class TToken;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FFFileSystemTokenTag;
//----------------------------------------------------------------------------
using FFileSystemToken = Core::TToken<
    FFFileSystemTokenTag,
    FileSystem::char_type,
    FileSystem::CaseSensitive,
    FileSystem::TTokenTraits,
    ALLOCATOR(FileSystem, FileSystem::char_type)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
