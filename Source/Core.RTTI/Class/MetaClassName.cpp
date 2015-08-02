#include "stdafx.h"

#include "MetaClassName.h"

namespace Core {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MetaClassName::MetaClassName(const RTTI::char_type* content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
MetaClassName::MetaClassName(const RTTI::char_type* content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
MetaClassName& MetaClassName::operator =(const RTTI::char_type* content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
MetaClassName::MetaClassName(const MetaClassName& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
MetaClassName& MetaClassName::operator =(const MetaClassName& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void MetaClassName::Swap(MetaClassName& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
