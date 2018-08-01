#pragma once

#include "Core.h"

#include "IO/FS/FileSystemToken.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_API FMountingPoint : public FFileSystemToken {
public:
    typedef FFileSystemToken parent_type;

    FMountingPoint() {}
    ~FMountingPoint() {}

    FMountingPoint(const FileSystem::FString& content);
    FMountingPoint& operator =(const FileSystem::FString& content);

    FMountingPoint(const FileSystem::FStringView& content);
    FMountingPoint& operator =(const FileSystem::FStringView& content);

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
} //!namespace PPE
