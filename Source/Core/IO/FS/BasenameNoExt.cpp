#include "stdafx.h"

#include "BasenameNoExt.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasenameNoExt::FBasenameNoExt(const FileSystem::FStringView& content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
FBasenameNoExt& FBasenameNoExt::operator =(const FileSystem::FStringView& content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
FBasenameNoExt::FBasenameNoExt(const FBasenameNoExt& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
FBasenameNoExt& FBasenameNoExt::operator =(const FBasenameNoExt& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
FBasenameNoExt::FBasenameNoExt(const FFileSystemToken& token)
:   parent_type(token) {}
//----------------------------------------------------------------------------
FBasenameNoExt& FBasenameNoExt::operator =(const FFileSystemToken& token) {
    parent_type::operator =(token);
    return *this;
}
//----------------------------------------------------------------------------
void FBasenameNoExt::Swap(FBasenameNoExt& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
