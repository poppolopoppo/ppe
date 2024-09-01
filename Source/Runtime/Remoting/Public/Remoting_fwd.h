#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_REMOTING
#   define PPE_REMOTING_API DLL_EXPORT
#else
#   define PPE_REMOTING_API DLL_IMPORT
#endif

#include "Memory/RefPtr.h"
#include "Misc/Function_fwd.h"
#include "Network_fwd.h"
#include "Memory/UniquePtr.h"
#include "Thread/Task_fwd.h"

namespace PPE {
FWD_INTEFARCE_UNIQUEPTR(RemotingService);
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FOpenAPI;
//----------------------------------------------------------------------------
class FRemotingServer;
//----------------------------------------------------------------------------
struct FRemotingContext;
class IRemotingEndpoint;
//----------------------------------------------------------------------------
using FRemotingRequest = Network::FHttpRequest;
using FRemotingResponse = Network::FHttpResponse;
using FRemotingCallback = TFunction< bool(const FRemotingServer&) NOEXCEPT >;
//----------------------------------------------------------------------------
FWD_REFPTR(BaseEndpoint);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
