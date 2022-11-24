// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
FMountingPoint::FMountingPoint(const FFileSystemToken& token) NOEXCEPT
:   parent_type(token) {
    Assert(token.empty() || L':' == token.MakeView().back());
}
//----------------------------------------------------------------------------
FMountingPoint& FMountingPoint::operator =(const FFileSystemToken& token) NOEXCEPT {
    Assert(token.empty() || L':' == token.MakeView().back());
    parent_type::operator =(token);
    return *this;
}
//----------------------------------------------------------------------------
void FMountingPoint::Swap(FMountingPoint& other) NOEXCEPT {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
