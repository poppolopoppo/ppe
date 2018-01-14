#include "stdafx.h"

#include "Extname.h"

#include "IO/String.h"
#include "IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FExtname::FExtname(const FileSystem::FString& content)
:   FExtname(content.MakeView())
{}
//----------------------------------------------------------------------------
FExtname& FExtname::operator =(const FileSystem::FString& content) {
    return operator =(content.MakeView());
}
//----------------------------------------------------------------------------
FExtname::FExtname(const FileSystem::FStringView& content)
:   parent_type(content) {
    Assert(content.size() && L'.' == content[0]);
}
//----------------------------------------------------------------------------
FExtname& FExtname::operator =(const FileSystem::FStringView& content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
FExtname::FExtname(const FExtname& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
FExtname& FExtname::operator =(const FExtname& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
FExtname::FExtname(const FFileSystemToken& token)
:   parent_type(token) {
    Assert(token.empty() || L':' == *token.c_str());
}
//----------------------------------------------------------------------------
FExtname& FExtname::operator =(const FFileSystemToken& token) {
    parent_type::operator =(token);
    Assert(token.empty() || L':' == *token.c_str());
    return *this;
}
//----------------------------------------------------------------------------
void FExtname::Swap(FExtname& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
