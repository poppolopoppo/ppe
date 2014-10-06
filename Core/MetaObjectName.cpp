#include "stdafx.h"

#include "MetaObjectName.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaObjectName::MetaObjectName(const RTTI::char_type* content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
MetaObjectName::MetaObjectName(const RTTI::char_type* content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
MetaObjectName& MetaObjectName::operator =(const RTTI::char_type* content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
MetaObjectName::MetaObjectName(const MetaObjectName& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
MetaObjectName& MetaObjectName::operator =(const MetaObjectName& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void MetaObjectName::Swap(MetaObjectName& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
