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
    Case            _Sensitive,
    typename        _TokenTraits,
    typename        _Allocator
>
class Token;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FileSystemTokenTag;
//----------------------------------------------------------------------------
using FileSystemToken = Core::Token<
    FileSystemTokenTag,
    FileSystem::char_type,
    FileSystem::CaseSensitive,
    FileSystem::TokenTraits,
    ALLOCATOR(FileSystem, FileSystem::char_type)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
