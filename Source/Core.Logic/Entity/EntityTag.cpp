#include "stdafx.h"

#include "EntityTag.h"

#include <locale>

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool EntityTagTokenTraits::IsAllowedChar(char ch) const {
    return IsAlnum(ch) || ch == '_' || ch == '-' || ch == '.';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EntityTag::EntityTag(const char *content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
EntityTag::EntityTag(const char *content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
EntityTag& EntityTag::operator =(const char *content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
EntityTag::EntityTag(const EntityTag& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
EntityTag& EntityTag::operator =(const EntityTag& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void EntityTag::Swap(EntityTag& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
