#pragma once

#include "Network.h"

#include "Container/Token.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNameTokenTraits {
public:
    const std::locale& Locale() const { return std::locale::classic(); }
    PPE_NETWORK_API bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
BASICTOKEN_CLASS_DECL(PPE_NETWORK_API, FName, char, ECase::Insensitive, FNameTokenTraits);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
