#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Container/Token.h"
#include "Core.RTTI/RTTIProperties.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class MetaClassName : public RTTI::Token<MetaClassName>  {
public:
    typedef RTTI::Token<MetaClassName> parent_type;

    MetaClassName() {}
    ~MetaClassName() {}

    MetaClassName(const RTTI::char_type* content, size_t length);
    MetaClassName(const MemoryView<const RTTI::char_type>& data)
        : MetaClassName(data.Pointer(), data.size()) {}

    MetaClassName(const RTTI::char_type* content);
    MetaClassName& operator =(const RTTI::char_type* content);

    template <typename _CharTraits, typename _Allocator>
    MetaClassName(const std::basic_string<typename RTTI::char_type, _CharTraits, _Allocator>& content)
        : MetaClassName(content.c_str(), content.size()) {}

    MetaClassName(const MetaClassName& other);
    MetaClassName& operator =(const MetaClassName& other);

    void Swap(MetaClassName& other);
};
//----------------------------------------------------------------------------
inline void swap(MetaClassName& lhs, MetaClassName& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const MetaClassName& name) {
    return name.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
