
#include "stdafx.h"

#include "RemotingServer.h"

#include "RemotingEndpoint.h"

#include "Http/Request.h"
#include "Http/Response.h"
#include "Http/Server.h"
#include "Http/Status.h"

#include "Thread/Task/CompletionPort.h"
#include "Thread/Task/TaskContext.h"
#include "Thread/ThreadContext.h"

#include "Container/HashMap.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"
#include "Thread/DeferredStream.h"

namespace PPE {
namespace Remoting {
EXTERN_LOG_CATEGORY(PPE_REMOTING_API, Remoting)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FRemotingContext::Failed(Network::EHttpStatus status, FString&& reason) const {
    LOG(Remoting, Error, L"request \"{0}\" failed with: {1} ({2})", Request.Uri(), status, reason);

    pResponse->SetStatus(status);
    pResponse->SetReason(std::move(reason));
}
//----------------------------------------------------------------------------
void FRemotingContext::BadRequest(const FStringView& what) const {
    Failed(Network::EHttpStatus::BadRequest, StringFormat("bad request: {0}", what));
}
//----------------------------------------------------------------------------
void FRemotingContext::ExpectationFailed(const FStringView& what) const {
    Failed(Network::EHttpStatus::ExpectationFailed, StringFormat("expectation failed: {0}", what));
}
//----------------------------------------------------------------------------
void FRemotingContext::NotFound(const FStringView& what) const {
    Failed(Network::EHttpStatus::NotFound, StringFormat("failed to find: {0}", what));
}
//----------------------------------------------------------------------------
void FRemotingContext::WaitForSync(FRemotingCallback&& callback) const NOEXCEPT {
    FCompletionPort cp;
    cp.Start(1);
    Sync.FireAndForget([&cp, &callback](const FRemotingServer& srv) {
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
    Assert_NoAssume(PathSeparator == path.front());
    auto sep = path.FindAfter(PathSeparator, path.begin()/* skip first */);

    const FStringView name = path.CutBefore(sep);
    if (not name.empty()) {
        READSCOPELOCK(_barrierRW);
        auto it = _endpoints.find(name);
        if (_endpoints.end() != it) {
            pEndpoint = it->second.get();
        }
    }

    FRemotingResponse response;
    response.HTTP_SetAccessControlAllowOrigin(MakeStringView("*")); // avoid CORS policy issues

    if (Likely(pEndpoint)) {
        response.SetStatus(Network::EHttpStatus::OK);
        pEndpoint->Process({ &response, request, _sync.Public() });
    }
    else {
        response.SetStatus(Network::EHttpStatus::Forbidden);
        response.SetReason("No endpoint found");
    }

    FRemotingResponse::Write(&port.Socket(), response);

    return request.AskToKeepAlive();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
