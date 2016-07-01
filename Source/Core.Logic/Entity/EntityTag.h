#pragma once

#include "Core.Logic/Logic.h"

#include "Core/Container/Token.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EntityTagTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
template <typename _Tag>
using EntityTagToken = Core::Token<
    _Tag,
    char,
    Case::Sensitive,
    EntityTagTokenTraits,
    ALLOCATOR(Entity, char)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class EntityTag : public EntityTagToken<EntityTag> {
public:
    typedef EntityTagToken<EntityTag> parent_type;

    EntityTag() {}
    ~EntityTag() {}

    EntityTag(const char *content, size_t length);

    EntityTag(const char *content);
    EntityTag& operator =(const char *content);

    template <typename _CharTraits, typename _Allocator>
    EntityTag(const std::basic_string<char, _CharTraits, _Allocator>& content)
        : EntityTag(content.c_str(), content.size()) {}

    EntityTag(const EntityTag& other);
    EntityTag& operator =(const EntityTag& other);

    void Swap(EntityTag& other);
};
//----------------------------------------------------------------------------
inline void swap(EntityTag& lhs, EntityTag& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const EntityTag& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
