#pragma once

#include "Core/Core.h"

#include "Core/Container/Token.h"
#include "Core.RTTI/RTTIProperties.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaPropertyName : public RTTI::Token<MetaPropertyName>  {
public:
    typedef RTTI::Token<MetaPropertyName> parent_type;

    MetaPropertyName() {}
    ~MetaPropertyName() {}

    MetaPropertyName(const RTTI::char_type* content, size_t length);

    MetaPropertyName(const RTTI::char_type* content);
    MetaPropertyName& operator =(const RTTI::char_type* content);

    template <typename _CharTraits, typename _Allocator>
    MetaPropertyName(const std::basic_string<typename RTTI::char_type, _CharTraits, _Allocator>& content)
        : MetaPropertyName(content.c_str(), content.size()) {}

    MetaPropertyName(const MetaPropertyName& other);
    MetaPropertyName& operator =(const MetaPropertyName& other);

    void Swap(MetaPropertyName& other);
};
//----------------------------------------------------------------------------
inline void swap(MetaPropertyName& lhs, MetaPropertyName& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline size_t hash_value(const MetaPropertyName& name) {
    return name.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
