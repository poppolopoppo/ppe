#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/StringSlice.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MountingPoint : public FileSystemToken {
public:
    typedef FileSystemToken parent_type;

    MountingPoint() {}
    ~MountingPoint() {}

    MountingPoint(const FileSystem::StringSlice& content);
    MountingPoint& operator =(const FileSystem::StringSlice& content);

    template <typename _CharTraits, typename _Allocator>
    MountingPoint(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : MountingPoint(MakeStringSlice(content)) {}

    MountingPoint(const MountingPoint& other);
    MountingPoint& operator =(const MountingPoint& other);

    MountingPoint(const FileSystemToken& token);
    MountingPoint& operator =(const FileSystemToken& token);

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
