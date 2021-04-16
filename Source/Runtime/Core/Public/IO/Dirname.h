#pragma once

#include "Core.h"

#include "IO/FileSystemToken.h"
#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FDirname : public FFileSystemToken {
public:
    typedef FFileSystemToken parent_type;

    FDirname() = default;

    FDirname(const FDirname& other) = default;
    FDirname& operator =(const FDirname& other) = default;

    FDirname(const FileSystem::FString& content);
    FDirname& operator =(const FileSystem::FString& content);

    FDirname(const FileSystem::FStringView& content);
    FDirname& operator =(const FileSystem::FStringView& content);

    template <size_t _Dim>
    FDirname(const FileSystem::char_type (&content)[_Dim]) : FDirname(MakeStringView(content)) {}
    template <size_t _Dim>
    FDirname& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    FDirname(const FFileSystemToken& token) NOEXCEPT;
    FDirname& operator =(const FFileSystemToken& token) NOEXCEPT;

    void Swap(FDirname& other) NOEXCEPT;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FDirname)
//----------------------------------------------------------------------------
inline void swap(FDirname& lhs, FDirname& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FDirname& token) NOEXCEPT {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
