#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Token.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMeshTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
template <typename _Tag>
using TMeshToken = Core::TToken<
    _Tag,
    char,
    CaseSensitive::False,
    FMeshTokenTraits,
    ALLOCATOR(FMaterial, char)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMeshName : public MeshToken<FMeshName> {
public:
    typedef MeshToken<FMeshName> parent_type;

    FMeshName() {}
    ~FMeshName() {}

    FMeshName(const char *content, size_t length);

    FMeshName(const char *content);
    FMeshName& operator =(const char *content);

    template <typename _CharTraits, typename _Allocator>
    FMeshName(const std::basic_string<char, _CharTraits, _Allocator>& content)
        : FMeshName(content.c_str(), content.size()) {}

    FMeshName(const FMeshName& other);
    FMeshName& operator =(const FMeshName& other);

    void Swap(FMeshName& other);
};
//----------------------------------------------------------------------------
inline void swap(FMeshName& lhs, FMeshName& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FMeshName& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
