#include "stdafx.h"

#include "EntityTag.h"

#include "IO/String.h"
#include "IO/StringView.h"

#include <locale>

namespace PPE {
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
FEntityTag::FEntityTag(const FString& content)
:   parent_type(content.data(), content.size())
{}
//----------------------------------------------------------------------------
FEntityTag::FEntityTag(const FStringView& content)
:   parent_type(content.data(), content.size())
{}
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
} //!namespace PPE
