#pragma once

#include "Graphics.h"

#include "Core/Token.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BindTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
template <typename _Tag>
using BindToken = Core::Token<
    _Tag,
    char,
    CaseSensitive::False,
    BindTokenTraits,
    ALLOCATOR(Material, char)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BindName : public BindToken<BindName> {
public:
    typedef BindToken<BindName> parent_type;

    BindName() {}
    ~BindName() {}

    BindName(const char *content, size_t length);

    BindName(const char *content);
    BindName& operator =(const char *content);

    template <typename _CharTraits, typename _Allocator>
    BindName(const std::basic_string<char, _CharTraits, _Allocator>& content)
        : BindName(content.c_str(), content.size()) {}

    BindName(const BindName& other);
    BindName& operator =(const BindName& other);

    void Swap(BindName& other);
};
//----------------------------------------------------------------------------
inline void swap(BindName& lhs, BindName& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline size_t hash_value(const BindName& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
