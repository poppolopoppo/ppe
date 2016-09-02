#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemToken.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Dirname : public FileSystemToken {
public:
    typedef FileSystemToken parent_type;

    Dirname() {}
    ~Dirname() {}

    Dirname(const FileSystem::StringView& content);
    Dirname& operator =(const FileSystem::StringView& content);

    template <size_t _Dim>
    Dirname(const FileSystem::char_type (&content)[_Dim]) : Dirname(MakeStringView(content)) {}
    template <size_t _Dim>
    Dirname& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    template <typename _CharTraits, typename _Allocator>
    Dirname(const std::basic_string<typename FileSystem::char_type, _CharTraits, _Allocator>& content)
        : Dirname(MakeStringView(content)) {}

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
inline hash_t hash_value(const Dirname& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
