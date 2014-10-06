#include "stdafx.h"

#include "BindName.h"

#include <locale>

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool BindTokenTraits::IsAllowedChar(char ch) const {
    return  std::isalnum(ch, Locale()) ||
            ch == '_' || ch == '-' || ch == ':' || ch == '.';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BindName::BindName(const char *content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
BindName::BindName(const char *content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
BindName& BindName::operator =(const char *content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
BindName::BindName(const BindName& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
BindName& BindName::operator =(const BindName& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void BindName::Swap(BindName& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
