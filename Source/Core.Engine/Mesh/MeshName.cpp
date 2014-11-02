#include "stdafx.h"

#include "MeshName.h"

#include <locale>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MeshTokenTraits::IsAllowedChar(char ch) const {
    return  std::isalnum(ch, Locale()) ||
            ch == '_' || ch == '-' || ch == '.';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
MeshName::MeshName(const char *content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
MeshName::MeshName(const char *content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
MeshName& MeshName::operator =(const char *content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
MeshName::MeshName(const MeshName& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
MeshName& MeshName::operator =(const MeshName& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void MeshName::Swap(MeshName& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
