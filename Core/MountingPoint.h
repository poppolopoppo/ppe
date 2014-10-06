#pragma once

#include "Core.h"
#include "FileSystemProperties.h"
#include "Token.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MountingPoint : public FileSystem::Token<MountingPoint> {
public:
    typedef FileSystem::Token<MountingPoint> parent_type;

    MountingPoint() {}
    ~MountingPoint() {}

    MountingPoint(const FileSystem::char_type* content, size_t length);

    MountingPoint(const FileSystem::char_type* content);
    MountingPoint& operator =(const FileSystem::char_type* content);

    template <typename _CharTraits, typename _Allocator>
    MountingPoint(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : MountingPoint(content.c_str(), content.size()) {}

    MountingPoint(const MountingPoint& other);
    MountingPoint& operator =(const MountingPoint& other);

    void Swap(MountingPoint& other);
};
//----------------------------------------------------------------------------
inline void swap(MountingPoint& lhs, MountingPoint& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
