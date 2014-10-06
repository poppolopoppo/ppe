#include "stdafx.h"

#include "MetaPropertyName.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaPropertyName::MetaPropertyName(const RTTI::char_type* content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
MetaPropertyName::MetaPropertyName(const RTTI::char_type* content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
MetaPropertyName& MetaPropertyName::operator =(const RTTI::char_type* content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
MetaPropertyName::MetaPropertyName(const MetaPropertyName& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
MetaPropertyName& MetaPropertyName::operator =(const MetaPropertyName& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void MetaPropertyName::Swap(MetaPropertyName& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
