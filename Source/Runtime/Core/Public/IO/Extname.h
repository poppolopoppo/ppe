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

    FExtname() {}
    ~FExtname() {}

    FExtname(const FileSystem::FString& content);
    FExtname& operator =(const FileSystem::FString& content);

    FExtname(const FileSystem::FStringView& content);
    FExtname& operator =(const FileSystem::FStringView& content);

    FExtname(const FExtname& other);
    FExtname& operator =(const FExtname& other);

    FExtname(const FFileSystemToken& token);
    FExtname& operator =(const FFileSystemToken& token);

    void Swap(FExtname& other);
};
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