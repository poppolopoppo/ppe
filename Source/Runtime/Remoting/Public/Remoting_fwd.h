#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RUNTIME_REMOTING
#   define PPE_REMOTING_API DLL_EXPORT
#else
#   define PPE_REMOTING_API DLL_IMPORT
#endif

#include "Network_fwd.h"
#include "Misc/Function_fwd.h"
#include "Thread/Task_fwd.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FRemotingServer;
//----------------------------------------------------------------------------
struct FRemotingContext;
class IRemotingEndpoint;
//----------------------------------------------------------------------------
using FRemotingRequest = Network::FHttpRequest;
using FRemotingResponse = Network::FHttpResponse;
using FRemotingCallback = TFunction< void(const FRemotingServer&) >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
