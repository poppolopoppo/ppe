#include "stdafx.h"

#include "MountingPoint.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MountingPoint::MountingPoint(const FileSystem::char_type* content)
:   parent_type(content) {
    Assert(content && L':' == content[Length(content) - 1]);
}
//----------------------------------------------------------------------------
MountingPoint::MountingPoint(const FileSystem::char_type* content, size_t length)
:   parent_type(content, length) {
    Assert(content && L':' == content[length - 1]);
}
//----------------------------------------------------------------------------
MountingPoint& MountingPoint::operator =(const FileSystem::char_type* content) {
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
void MountingPoint::Swap(MountingPoint& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
