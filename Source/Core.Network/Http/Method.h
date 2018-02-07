#pragma once

#include "Core.Network/Network.h"

#include "Core/IO/String_fwd.h"
#include "Core/IO/TextWriter_fwd.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EHttpMethod {
    Get = 0,
    Head,
    Post,
    Put,
    Delete,
    Trace,
    Options,
    Connect,
    Patch,
};
//----------------------------------------------------------------------------
CORE_NETWORK_API FStringView HttpMethodToCStr(EHttpMethod method);
CORE_NETWORK_API bool HttpMethodFromCStr(EHttpMethod* method, const FStringView& str);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_NETWORK_API FTextWriter& operator <<(FTextWriter& oss, Network::EHttpMethod httpMethod);
CORE_NETWORK_API FWTextWriter& operator <<(FWTextWriter& oss, Network::EHttpMethod httpMethod);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
