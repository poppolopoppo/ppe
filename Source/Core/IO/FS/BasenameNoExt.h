#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BasenameNoExt : public FileSystemToken {
public:
    typedef FileSystemToken parent_type;

    BasenameNoExt() {}
    ~BasenameNoExt() {}

    BasenameNoExt(const FileSystem::StringView& content);
    BasenameNoExt& operator =(const FileSystem::StringView& content);

    template <typename _CharTraits, typename _Allocator>
    BasenameNoExt(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : BasenameNoExt(content.c_str(), content.size()) {}

    BasenameNoExt(const FileSystemToken& token);
    BasenameNoExt& operator =(const FileSystemToken& token);

    BasenameNoExt(const BasenameNoExt& other);
    BasenameNoExt& operator =(const BasenameNoExt& other);

    void Swap(BasenameNoExt& other);
};
//----------------------------------------------------------------------------
inline void swap(BasenameNoExt& lhs, BasenameNoExt& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const BasenameNoExt& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
