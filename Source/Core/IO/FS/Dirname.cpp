#include "stdafx.h"

#include "Dirname.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Dirname::Dirname(const FileSystem::StringSlice& content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
Dirname& Dirname::operator =(const FileSystem::StringSlice& content) {
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
Dirname::Dirname(const FileSystemToken& token)
:   parent_type(token) {}
//----------------------------------------------------------------------------
Dirname& Dirname::operator =(const FileSystemToken& token) {
    parent_type::operator =(token);
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
