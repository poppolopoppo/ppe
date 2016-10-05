#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMountingPoint : public FFileSystemToken {
public:
    typedef FFileSystemToken parent_type;

    FMountingPoint() {}
    ~FMountingPoint() {}

    FMountingPoint(const FileSystem::FStringView& content);
    FMountingPoint& operator =(const FileSystem::FStringView& content);

    template <typename _CharTraits, typename _Allocator>
    FMountingPoint(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : FMountingPoint(MakeStringView(content)) {}

    FMountingPoint(const FMountingPoint& other);
    FMountingPoint& operator =(const FMountingPoint& other);

    FMountingPoint(const FFileSystemToken& token);
    FMountingPoint& operator =(const FFileSystemToken& token);

    void Swap(FMountingPoint& other);
};
//----------------------------------------------------------------------------
inline void swap(FMountingPoint& lhs, FMountingPoint& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
