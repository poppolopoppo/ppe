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

    FBasenameNoExt() = default;

    FBasenameNoExt(const FBasenameNoExt& other) = default;
    FBasenameNoExt& operator =(const FBasenameNoExt& other) = default;

    FBasenameNoExt(const FileSystem::FStringView& content);
    FBasenameNoExt& operator =(const FileSystem::FStringView& content);

    FBasenameNoExt(const FileSystem::FString& content);

    FBasenameNoExt(const FFileSystemToken& token) NOEXCEPT;
    FBasenameNoExt& operator =(const FFileSystemToken& token) NOEXCEPT;

    void Clear() NOEXCEPT;
    void Swap(FBasenameNoExt& other) NOEXCEPT;

};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FBasenameNoExt)
//----------------------------------------------------------------------------
PPE_CORE_API FBasenameNoExt operator +(const FBasenameNoExt& lhs, const FileSystem::FStringView& rhs);
//----------------------------------------------------------------------------
inline void swap(FBasenameNoExt& lhs, FBasenameNoExt& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FBasenameNoExt& token) NOEXCEPT {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
