#include "stdafx.h"

#include "Extname.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Extname::Extname(const FileSystem::StringView& content)
:   parent_type(content) {
    Assert(content.size() && L'.' == content[0]);
}
//----------------------------------------------------------------------------
Extname& Extname::operator =(const FileSystem::StringView& content) {
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
Extname::Extname(const FileSystemToken& token)
:   parent_type(token) {
    Assert(token.empty() || L':' == *token.c_str());
}
//----------------------------------------------------------------------------
Extname& Extname::operator =(const FileSystemToken& token) {
    parent_type::operator =(token);
    Assert(token.empty() || L':' == *token.c_str());
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
