#pragma once

#include "Core/Core.h"

#include "Core/Container/Token.h"
#include "Core/IO/FS/FileSystemProperties.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Extname : public FileSystem::Token<Extname> {
public:
    typedef FileSystem::Token<Extname> parent_type;

    Extname() {}
    ~Extname() {}

    Extname(const FileSystem::char_type* content, size_t length);

    Extname(const FileSystem::char_type* content);
    Extname& operator =(const FileSystem::char_type* content);

    template <typename _CharTraits, typename _Allocator>
    Extname(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : Extname(content.c_str(), content.size()) {}

    Extname(const Extname& other);
    Extname& operator =(const Extname& other);

    void Swap(Extname& other);
};
//----------------------------------------------------------------------------
inline void swap(Extname& lhs, Extname& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline size_t hash_value(const Extname& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
