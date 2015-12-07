#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/Token.h"
#include "Core.RTTI/RTTIProperties.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaObjectName : public RTTI::Token<MetaObjectName>  {
public:
    typedef RTTI::Token<MetaObjectName> parent_type;

    MetaObjectName() {}
    ~MetaObjectName() {}

    MetaObjectName(const RTTI::char_type* content, size_t length);
    MetaObjectName(const MemoryView<const RTTI::char_type>& data)
        : MetaObjectName(data.Pointer(), data.size()) {}

    MetaObjectName(const RTTI::char_type* content);
    MetaObjectName& operator =(const RTTI::char_type* content);

    template <typename _CharTraits, typename _Allocator>
    MetaObjectName(const std::basic_string<typename RTTI::char_type, _CharTraits, _Allocator>& content)
        : MetaObjectName(content.c_str(), content.size()) {}

    MetaObjectName(const MetaObjectName& other);
    MetaObjectName& operator =(const MetaObjectName& other);

    void Swap(MetaObjectName& other);
};
//----------------------------------------------------------------------------
inline void swap(MetaObjectName& lhs, MetaObjectName& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const MetaObjectName& name) {
    return name.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
