#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBasenameNoExt : public FFileSystemToken {
public:
    typedef FFileSystemToken parent_type;

    FBasenameNoExt() {}
    ~FBasenameNoExt() {}

    FBasenameNoExt(const FileSystem::FStringView& content);
    FBasenameNoExt& operator =(const FileSystem::FStringView& content);

    template <typename _CharTraits, typename _Allocator>
    FBasenameNoExt(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : FBasenameNoExt(content.c_str(), content.size()) {}

    FBasenameNoExt(const FFileSystemToken& token);
    FBasenameNoExt& operator =(const FFileSystemToken& token);

    FBasenameNoExt(const FBasenameNoExt& other);
    FBasenameNoExt& operator =(const FBasenameNoExt& other);

    void Swap(FBasenameNoExt& other);
};
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
} //!namespace Core
