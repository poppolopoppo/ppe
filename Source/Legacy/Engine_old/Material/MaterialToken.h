#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/Allocation.h"
#include "Core/IO/String.h"

namespace Core {
template <
    typename        _Tag,
    typename        _Char,
    CaseSensitive   _Sensitive,
    typename        _TokenTraits,
    typename        _Allocator
>
class EToken;

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMaterialTokenTraits {
public:
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
template <typename _Tag>
using MaterialToken = PPE::EToken<
    _Tag,
    char,
    CaseSensitive::False,
    FMaterialTokenTraits,
    ALLOCATOR(FMaterial, char)
>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
