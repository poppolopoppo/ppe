#pragma once

#include "Core/Core.h"

#include "Core/Container/Token.h"
#include "Core/IO/FS/FileSystemProperties.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BasenameNoExt : public FileSystem::Token<BasenameNoExt> {
public:
    typedef FileSystem::Token<BasenameNoExt> parent_type;

    BasenameNoExt() {}
    ~BasenameNoExt() {}

    BasenameNoExt(const FileSystem::char_type* content, size_t length);

    BasenameNoExt(const FileSystem::char_type* content);
    BasenameNoExt& operator =(const FileSystem::char_type* content);

    template <typename _CharTraits, typename _Allocator>
    BasenameNoExt(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : BasenameNoExt(content.c_str(), content.size()) {}

    BasenameNoExt(const BasenameNoExt& other);
    BasenameNoExt& operator =(const BasenameNoExt& other);

    void Swap(BasenameNoExt& other);
};
//----------------------------------------------------------------------------
inline void swap(BasenameNoExt& lhs, BasenameNoExt& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline size_t hash_value(const BasenameNoExt& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
