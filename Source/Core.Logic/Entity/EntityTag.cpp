#include "stdafx.h"

#include "EntityTag.h"

#include <locale>

namespace Core {
namespace Logic {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FEntityTagTokenTraits::IsAllowedChar(char ch) const {
    return IsAlnum(ch) || ch == '_' || ch == '-' || ch == '.';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FEntityTag::FEntityTag(const char *content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
FEntityTag::FEntityTag(const char *content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
FEntityTag& FEntityTag::operator =(const char *content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
FEntityTag::FEntityTag(const FEntityTag& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
FEntityTag& FEntityTag::operator =(const FEntityTag& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void FEntityTag::Swap(FEntityTag& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Logic
} //!namespace Core
