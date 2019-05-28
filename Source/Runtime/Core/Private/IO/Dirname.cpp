#include "stdafx.h"

#include "IO/Dirname.h"

#include "IO/String.h"
#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDirname::FDirname(const FileSystem::FStringView& content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
FDirname::FDirname(const FileSystem::FString& content)
:   parent_type(content.MakeView())
{}
//----------------------------------------------------------------------------
FDirname& FDirname::operator =(const FileSystem::FString& content) {
    return operator =(content.MakeView());
}
//----------------------------------------------------------------------------
FDirname& FDirname::operator =(const FileSystem::FStringView& content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
FDirname::FDirname(const FFileSystemToken& token) NOEXCEPT
:   parent_type(token) {}
//----------------------------------------------------------------------------
FDirname& FDirname::operator =(const FFileSystemToken& token) NOEXCEPT {
    parent_type::operator =(token);
    return *this;
}
//----------------------------------------------------------------------------
void FDirname::Swap(FDirname& other) NOEXCEPT {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
