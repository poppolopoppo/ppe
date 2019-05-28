#pragma once

#include "Core.h"

#include "IO/FileSystemToken.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FMountingPoint : public FFileSystemToken {
public:
    typedef FFileSystemToken parent_type;

    FMountingPoint() = default;

    FMountingPoint(const FMountingPoint& other) = default;
    FMountingPoint& operator =(const FMountingPoint& other) = default;

    FMountingPoint(const FileSystem::FString& content);
    FMountingPoint& operator =(const FileSystem::FString& content);

    FMountingPoint(const FileSystem::FStringView& content);
    FMountingPoint& operator =(const FileSystem::FStringView& content);

    FMountingPoint(const FFileSystemToken& token) NOEXCEPT;
    FMountingPoint& operator =(const FFileSystemToken& token) NOEXCEPT;

    void Swap(FMountingPoint& other) NOEXCEPT;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FMountingPoint)
//----------------------------------------------------------------------------
inline void swap(FMountingPoint& lhs, FMountingPoint& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
