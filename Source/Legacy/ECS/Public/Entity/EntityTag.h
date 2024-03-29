#pragma once

#include "Core.Logic/Logic.h"

#include "Container/Token.h"

namespace PPE {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FEntityTagTokenTraits {
public:
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
template <typename _Tag>
using EntityTagToken = PPE::TToken<
    _Tag,
    char,
    ECase::Sensitive,
    FEntityTagTokenTraits
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_LOGIC_API FEntityTag : public EntityTagToken<FEntityTag> {
public:
    typedef EntityTagToken<FEntityTag> parent_type;

    FEntityTag() {}
    ~FEntityTag() {}

    FEntityTag(const FString& content);
    FEntityTag(const FStringView& str);
    FEntityTag(const char *content, size_t length);

    FEntityTag(const char *content);
    FEntityTag& operator =(const char *content);

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
} //!namespace PPE
