#pragma once

#include "Network_fwd.h"

#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
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
PPE_NETWORK_API FStringView HttpMethodToCStr(EHttpMethod method);
PPE_NETWORK_API bool HttpMethodFromCStr(EHttpMethod* method, const FStringView& str);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_NETWORK_API FTextWriter& operator <<(FTextWriter& oss, Network::EHttpMethod httpMethod);
PPE_NETWORK_API FWTextWriter& operator <<(FWTextWriter& oss, Network::EHttpMethod httpMethod);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
