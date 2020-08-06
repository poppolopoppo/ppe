
#include "stdafx.h"

#include "RemotingServer.h"

#include "RemotingEndpoint.h"
#include "Container/HashMap.h"

#include "Http/Request.h"
#include "Http/Response.h"
#include "Http/Server.h"
#include "Http/Status.h"

#include "Thread/Task/CompletionPort.h"
#include "Thread/Task/TaskContext.h"
#include "Thread/ThreadContext.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FRemotingContext::WaitForSync(FRemotingCallback&& callback) const NOEXCEPT {
    FCompletionPort cp;
    cp.Start(1);
    Sync.Emplace([&cp, &callback](const FRemotingServer& srv) {
        callback(srv);
        cp.OnJobComplete(); // resume remoting job on worker thread
    });
    ITaskContext::Get().WaitFor(cp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRemotingServer::FRemotingServer() NOEXCEPT
:   Network::FHttpServer("Remoting", Network::FAddress::Localhost(1985))
{}
//----------------------------------------------------------------------------
FRemotingServer::~FRemotingServer() NOEXCEPT {
    Assert_NoAssume(not _sync); // should have already flush all callbacks :/
}
//----------------------------------------------------------------------------
void FRemotingServer::Add(URemotingEndpoint&& endpoint) {
    Assert(endpoint);

    WRITESCOPELOCK(_barrierRW);

    _endpoints.Insert_AssertUnique(endpoint->Path(), std::move(endpoint));
}
//----------------------------------------------------------------------------
void FRemotingServer::Start() {
    AssertIsMainThread();

    Network::FHttpServer::Start(2);
}
//----------------------------------------------------------------------------
void FRemotingServer::Tick(FTimespan) {
    AssertIsMainThread();

    _sync.FireAndForget(*this);
}
//----------------------------------------------------------------------------
void FRemotingServer::Shutdown() {
    AssertIsMainThread();

    Network::FHttpServer::Stop();
}
//----------------------------------------------------------------------------
bool FRemotingServer::OnRequest(Network::FServicingPort& port, const FRemotingRequest& request) const {
    static constexpr auto PathSeparator = Network::FUri::PathSeparator;

    IRemotingEndpoint* pEndpoint = nullptr;

    const FStringView& path = request.Uri().Path();
    auto sep = path.Find(PathSeparator);

    for (;;) {
        const FStringView name = path.CutBefore(sep);
        {
            READSCOPELOCK(_barrierRW);
            auto it = _endpoints.find(name);
            if (_endpoints.end() != it) {
                pEndpoint = it->second.get();
                break;
            }
        }
        sep = path.FindAfter(PathSeparator, sep);
    }

    FRemotingResponse response;
    if (Likely(pEndpoint)) {
        pEndpoint->Process(FRemotingContext{
            &response, request, _sync.Public()
        });
    }
    else {
        response.SetStatus(Network::EHttpStatus::ServiceUnavailable);
        response.SetReason("No endpoint found");
    }

    FRemotingResponse::Write(&port.Socket(), response);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
