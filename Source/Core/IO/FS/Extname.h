#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/String_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FExtname : public FFileSystemToken {
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
} //!namespace Core
