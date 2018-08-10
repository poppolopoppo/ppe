#include "stdafx.h"

#include "IO/MountingPoint.h"

#include "IO/String.h"
#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMountingPoint::FMountingPoint(const FileSystem::FString& content)
:   FMountingPoint(content.MakeView())
{}
//----------------------------------------------------------------------------
FMountingPoint& FMountingPoint::operator =(const FileSystem::FString& content) {
    return operator =(content.MakeView());
}
//----------------------------------------------------------------------------
FMountingPoint::FMountingPoint(const FileSystem::FStringView& content)
:   parent_type(content) {
    Assert(content.size() && L':' == content.back());
}
//----------------------------------------------------------------------------
FMountingPoint& FMountingPoint::operator =(const FileSystem::FStringView& content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
FMountingPoint::FMountingPoint(const FMountingPoint& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
FMountingPoint& FMountingPoint::operator =(const FMountingPoint& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
FMountingPoint::FMountingPoint(const FFileSystemToken& token)
:   parent_type(token) {
    Assert(token.empty() || L':' == token.MakeView().back());
}
//----------------------------------------------------------------------------
FMountingPoint& FMountingPoint::operator =(const FFileSystemToken& token) {
    Assert(token.empty() || L':' == token.MakeView().back());
    parent_type::operator =(token);
    return *this;
}
//----------------------------------------------------------------------------
void FMountingPoint::Swap(FMountingPoint& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
