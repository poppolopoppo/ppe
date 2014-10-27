#include "stdafx.h"

#include "Dirname.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Dirname::Dirname(const FileSystem::char_type* content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
Dirname::Dirname(const FileSystem::char_type* content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
Dirname& Dirname::operator =(const FileSystem::char_type* content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
Dirname::Dirname(const Dirname& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
Dirname& Dirname::operator =(const Dirname& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void Dirname::Swap(Dirname& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
