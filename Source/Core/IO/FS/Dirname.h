#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/StringSlice.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Dirname : public FileSystemToken {
public:
    typedef FileSystemToken parent_type;

    Dirname() {}
    ~Dirname() {}

    Dirname(const FileSystem::char_type* content, size_t length);

    Dirname(const FileSystem::char_type* content);
    Dirname& operator =(const FileSystem::char_type* content);

    template <typename _CharTraits, typename _Allocator>
    Dirname(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : Dirname(content.c_str(), content.size()) {}

    Dirname(const BasicStringSlice<const FileSystem::char_type>& slice)
        : Dirname(slice.Pointer(), slice.size()) {}

    Dirname(const FileSystemToken& token);
    Dirname& operator =(const FileSystemToken& token);

    Dirname(const Dirname& other);
    Dirname& operator =(const Dirname& other);

    void Swap(Dirname& other);
};
//----------------------------------------------------------------------------
inline void swap(Dirname& lhs, Dirname& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline size_t hash_value(const Dirname& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
