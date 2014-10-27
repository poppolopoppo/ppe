#include "stdafx.h"

#include "BasenameNoExt.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BasenameNoExt::BasenameNoExt(const FileSystem::char_type* content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
BasenameNoExt::BasenameNoExt(const FileSystem::char_type* content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
BasenameNoExt& BasenameNoExt::operator =(const FileSystem::char_type* content) {
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
void BasenameNoExt::Swap(BasenameNoExt& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
