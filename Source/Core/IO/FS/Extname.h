#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FExtname : public FFileSystemToken {
public:
    typedef FFileSystemToken parent_type;

    FExtname() {}
    ~FExtname() {}

    FExtname(const FileSystem::FStringView& content);
    FExtname& operator =(const FileSystem::FStringView& content);

    template <typename _CharTraits, typename _Allocator>
    FExtname(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : FExtname(content.c_str(), content.size()) {}

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
