#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Container/Token.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class NameTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
BASICTOKEN_CLASS_DEF(Name, char, Case::Insensitive, NameTokenTraits);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
