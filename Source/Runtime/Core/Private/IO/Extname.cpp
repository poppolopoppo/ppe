#include "stdafx.h"

#include "IO/Extname.h"

#include "IO/String.h"
#include "IO/StringView.h"

namespace PPE {
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
    Assert(content.empty() || L'.' == content.front());
}
//----------------------------------------------------------------------------
FExtname& FExtname::operator =(const FileSystem::FStringView& content) {
    Assert(content.empty() || L'.' == content.front());
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
    : parent_type(token) {
    Assert(token.empty() || L'.' == *token.c_str());
}
//----------------------------------------------------------------------------
FExtname& FExtname::operator =(const FFileSystemToken& token) {
    Assert(token.empty() || L'.' == *token.c_str());
    parent_type::operator =(token);
    return *this;
}
//----------------------------------------------------------------------------
void FExtname::Swap(FExtname& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE