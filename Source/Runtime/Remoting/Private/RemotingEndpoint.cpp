#include "stdafx.h"

#include "RemotingEndpoint.h"

#include "RemotingServer.h"

#include "Diagnostic/Logger.h"
#include "Http/Request.h"
#include "Uri.h"

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

    LOG(Remoting, Debug, L"Call endpoint <{0}> for '{1}'", _path, ctx.Request.Uri());

    Assert_NoAssume(StartsWithI(ctx.Request.Uri().Path(), _path));
    FStringView relativePath = ctx.Request.Uri().Path().CutStartingAt(_path.length());
    if (relativePath.StartsWith(Network::FUri::PathSeparator))
        relativePath = relativePath.ShiftFront();

    // #TODO : benchmarking histogram for performance
    ProcessImpl(ctx, relativePath);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
