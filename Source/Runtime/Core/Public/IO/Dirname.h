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

    FDirname() {}
    ~FDirname() {}

    FDirname(const FileSystem::FString& content);
    FDirname& operator =(const FileSystem::FString& content);

    FDirname(const FileSystem::FStringView& content);
    FDirname& operator =(const FileSystem::FStringView& content);

    template <size_t _Dim>
    FDirname(const FileSystem::char_type (&content)[_Dim]) : FDirname(MakeStringView(content)) {}
    template <size_t _Dim>
    FDirname& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    FDirname(const FFileSystemToken& token);
    FDirname& operator =(const FFileSystemToken& token);

    FDirname(const FDirname& other);
    FDirname& operator =(const FDirname& other);

    void Swap(FDirname& other);
};
//----------------------------------------------------------------------------
inline void swap(FDirname& lhs, FDirname& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FDirname& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
