#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_NETWORK
#   define PPE_NETWORK_API DLL_EXPORT
#else
#   define PPE_NETWORK_API DLL_IMPORT
#endif

#include "Memory/RefPtr.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FUri;
class FAddress;
enum class EServiceName : size_t;
//----------------------------------------------------------------------------
class FListener;
class FSocketBuffered;
FWD_REFPTR(HandShaker);
FWD_REFPTR(ServicingPort);
//----------------------------------------------------------------------------
class FHttpException;
class FHttpRequest;
class FHttpResponse;
enum class EHttpMethod;
enum class EHttpStatus;
//----------------------------------------------------------------------------
class FHttpClient;
class FHttpServer;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
