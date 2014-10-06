#include "stdafx.h"

#include "Extname.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Extname::Extname(const FileSystem::char_type* content)
:   parent_type(content) {
    Assert(content && L'.' == content[0]);
}
//----------------------------------------------------------------------------
Extname::Extname(const FileSystem::char_type* content, size_t length)
:   parent_type(content, length) {
    Assert(content && L'.' == content[0]);
}
//----------------------------------------------------------------------------
Extname& Extname::operator =(const FileSystem::char_type* content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
Extname::Extname(const Extname& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
Extname& Extname::operator =(const Extname& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void Extname::Swap(Extname& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
