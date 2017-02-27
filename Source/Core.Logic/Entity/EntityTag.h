#pragma once

#include "Core.Logic/Logic.h"

#include "Core/Container/Token.h"

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEntityTagTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
template <typename _Tag>
using EntityTagToken = Core::TToken<
    _Tag,
    char,
    ECase::Sensitive,
    FEntityTagTokenTraits,
    ALLOCATOR(FEntity, char)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEntityTag : public EntityTagToken<FEntityTag> {
public:
    typedef EntityTagToken<FEntityTag> parent_type;

    FEntityTag() {}
    ~FEntityTag() {}

    FEntityTag(const char *content, size_t length);

    FEntityTag(const char *content);
    FEntityTag& operator =(const char *content);

    template <typename _CharTraits, typename _Allocator>
    FEntityTag(const std::basic_string<char, _CharTraits, _Allocator>& content)
        : FEntityTag(content.c_str(), content.size()) {}

    FEntityTag(const FEntityTag& other);
    FEntityTag& operator =(const FEntityTag& other);

    void Swap(FEntityTag& other);
};
//----------------------------------------------------------------------------
inline void swap(FEntityTag& lhs, FEntityTag& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FEntityTag& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
