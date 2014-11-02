#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Token.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MeshTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
template <typename _Tag>
using MeshToken = Core::Token<
    _Tag,
    char,
    CaseSensitive::False,
    MeshTokenTraits,
    ALLOCATOR(Material, char)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MeshName : public MeshToken<MeshName> {
public:
    typedef MeshToken<MeshName> parent_type;

    MeshName() {}
    ~MeshName() {}

    MeshName(const char *content, size_t length);

    MeshName(const char *content);
    MeshName& operator =(const char *content);

    template <typename _CharTraits, typename _Allocator>
    MeshName(const std::basic_string<char, _CharTraits, _Allocator>& content)
        : MeshName(content.c_str(), content.size()) {}

    MeshName(const MeshName& other);
    MeshName& operator =(const MeshName& other);

    void Swap(MeshName& other);
};
//----------------------------------------------------------------------------
inline void swap(MeshName& lhs, MeshName& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline size_t hash_value(const MeshName& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
