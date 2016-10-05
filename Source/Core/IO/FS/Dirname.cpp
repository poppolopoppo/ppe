#include "stdafx.h"

#include "Dirname.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDirname::FDirname(const FileSystem::FStringView& content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
FDirname& FDirname::operator =(const FileSystem::FStringView& content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
FDirname::FDirname(const FDirname& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
FDirname& FDirname::operator =(const FDirname& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
FDirname::FDirname(const FFileSystemToken& token)
:   parent_type(token) {}
//----------------------------------------------------------------------------
FDirname& FDirname::operator =(const FFileSystemToken& token) {
    parent_type::operator =(token);
    return *this;
}
//----------------------------------------------------------------------------
void FDirname::Swap(FDirname& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
