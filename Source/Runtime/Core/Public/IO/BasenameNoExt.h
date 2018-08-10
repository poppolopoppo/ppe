#pragma once

#include "Core.h"

#include "IO/FileSystemToken.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FBasenameNoExt : public FFileSystemToken {
public:
    typedef FFileSystemToken parent_type;

    FBasenameNoExt() {}
    ~FBasenameNoExt() {}

    FBasenameNoExt(const FileSystem::FStringView& content);
    FBasenameNoExt& operator =(const FileSystem::FStringView& content);

    FBasenameNoExt(const FileSystem::FString& content);

    FBasenameNoExt(const FFileSystemToken& token);
    FBasenameNoExt& operator =(const FFileSystemToken& token);

    FBasenameNoExt(const FBasenameNoExt& other);
    FBasenameNoExt& operator =(const FBasenameNoExt& other);

    void Swap(FBasenameNoExt& other);
};
//----------------------------------------------------------------------------
PPE_CORE_API FBasenameNoExt operator +(const FBasenameNoExt& lhs, const FileSystem::FStringView& rhs);
//----------------------------------------------------------------------------
inline void swap(FBasenameNoExt& lhs, FBasenameNoExt& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FBasenameNoExt& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
