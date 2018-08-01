#include "stdafx.h"

#include "MeshName.h"

#include <locale>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FMeshTokenTraits::IsAllowedChar(char ch) const {
    return IsAlnum(ch) || ch == '_' || ch == '-' || ch == '.';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMeshName::FMeshName(const char *content)
:   parent_type(content) {}
//----------------------------------------------------------------------------
FMeshName::FMeshName(const char *content, size_t length)
:   parent_type(content, length) {}
//----------------------------------------------------------------------------
FMeshName& FMeshName::operator =(const char *content) {
    parent_type::operator =(content);
    return *this;
}
//----------------------------------------------------------------------------
FMeshName::FMeshName(const FMeshName& other)
:   parent_type(other) {}
//----------------------------------------------------------------------------
FMeshName& FMeshName::operator =(const FMeshName& other) {
    parent_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
void FMeshName::Swap(FMeshName& other) {
    parent_type::Swap(other);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
