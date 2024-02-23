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
PPE_NETWORK_API FStringLiteral HttpMethodToCStr(EHttpMethod method);
PPE_NETWORK_API bool HttpMethodFromCStr(EHttpMethod* method, const FStringView& str);
//----------------------------------------------------------------------------
PPE_NETWORK_API FTextWriter& operator <<(FTextWriter& oss, EHttpMethod httpMethod);
PPE_NETWORK_API FWTextWriter& operator <<(FWTextWriter& oss, EHttpMethod httpMethod);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
