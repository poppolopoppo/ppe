#include "stdafx.h"

#include "MountingPoint.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MountingPoint::MountingPoint(const FileSystem::StringView& content)
:   parent_type(content) {
    Assert(content.size() && L':' == content.back());
}
//----------------------------------------------------------------------------
MountingPoint& MountingPoint::operator =(const FileSystem::StringView& content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
MountingPoint::MountingPoint(const MountingPoint& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
MountingPoint& MountingPoint::operator =(const MountingPoint& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
MountingPoint::MountingPoint(const FileSystemToken& token)
:   parent_type(token) {
    Assert(token.empty() || L':' == token.MakeView().back());
}
//----------------------------------------------------------------------------
MountingPoint& MountingPoint::operator =(const FileSystemToken& token) {
    Assert(token.empty() || L':' == token.MakeView().back());
    parent_type::operator =(token);
    return *this;
}
//----------------------------------------------------------------------------
void MountingPoint::Swap(MountingPoint& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
