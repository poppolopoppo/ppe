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
    PPE_NETWORK_API bool IsAllowedChar(char ch) const;
};
//----------------------------------------------------------------------------
BASICTOKEN_CLASS_DECL(PPE_NETWORK_API, Name, char, ECase::Insensitive, FNameTokenTraits);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Network::FName operator ""_network (const char* str, size_t len) {
    return Network::FName{ FStringView(str, len) };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
