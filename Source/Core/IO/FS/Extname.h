#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Extname : public FileSystemToken {
public:
    typedef FileSystemToken parent_type;

    Extname() {}
    ~Extname() {}

    Extname(const FileSystem::StringView& content);
    Extname& operator =(const FileSystem::StringView& content);

    template <typename _CharTraits, typename _Allocator>
    Extname(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : Extname(content.c_str(), content.size()) {}

    Extname(const Extname& other);
    Extname& operator =(const Extname& other);

    Extname(const FileSystemToken& token);
    Extname& operator =(const FileSystemToken& token);

    void Swap(Extname& other);
};
//----------------------------------------------------------------------------
inline void swap(Extname& lhs, Extname& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const Extname& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
