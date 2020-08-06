#include "stdafx.h"

#include "RemotingEndpoint.h"

#include "RemotingServer.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace Remoting {
EXTERN_LOG_CATEGORY(PPE_REMOTING_API, Remoting)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
IRemotingEndpoint::IRemotingEndpoint(FString&& path) NOEXCEPT
:    _path(std::move(path))
{}
//----------------------------------------------------------------------------
IRemotingEndpoint::~IRemotingEndpoint() = default;
//----------------------------------------------------------------------------
void IRemotingEndpoint::Process(const FRemotingContext& ctx) {
    Assert(ctx.pResponse);

    LOG(Remoting, Debug, L"Call endpoint <{0}>", _path);

    // #TODO : benchmarking histogram for performance
    ProcessImpl(ctx);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
