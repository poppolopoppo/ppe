#include "stdafx.h"

#include "BasenameNoExt.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BasenameNoExt::BasenameNoExt(const FileSystem::StringSlice& content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
BasenameNoExt& BasenameNoExt::operator =(const FileSystem::StringSlice& content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
BasenameNoExt::BasenameNoExt(const BasenameNoExt& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
BasenameNoExt& BasenameNoExt::operator =(const BasenameNoExt& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
BasenameNoExt::BasenameNoExt(const FileSystemToken& token)
:   parent_type(token) {}
//----------------------------------------------------------------------------
BasenameNoExt& BasenameNoExt::operator =(const FileSystemToken& token) {
    parent_type::operator =(token);
    return *this;
}
//----------------------------------------------------------------------------
void BasenameNoExt::Swap(BasenameNoExt& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
