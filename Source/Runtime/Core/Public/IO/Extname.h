#pragma once

#include "Core.h"

#include "IO/FileSystemToken.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FExtname : public FFileSystemToken {
public:
    typedef FFileSystemToken parent_type;

    FExtname() = default;

    FExtname(const FExtname& other) = default;
    FExtname& operator =(const FExtname& other) = default;

    FExtname(const FileSystem::FString& content);
    FExtname& operator =(const FileSystem::FString& content);

    FExtname(const FileSystem::FStringView& content);
    FExtname& operator =(const FileSystem::FStringView& content);

    FExtname(const FFileSystemToken& token) NOEXCEPT;
    FExtname& operator =(const FFileSystemToken& token) NOEXCEPT;

    void Swap(FExtname& other) NOEXCEPT;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FExtname)
//----------------------------------------------------------------------------
inline void swap(FExtname& lhs, FExtname& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FExtname& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
