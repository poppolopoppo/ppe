// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RemotingServer.h"

#include "RemotingEndpoint.h"
#include "Remoting/BaseEndpoint.h"

#include "Http/Request.h"
#include "Http/Response.h"
#include "Http/Server.h"
#include "Http/Status.h"

#include "Thread/Task/CompletionPort.h"
#include "Thread/Task/TaskContext.h"
#include "Thread/ThreadContext.h"

#include "Diagnostic/Logger.h"
#include "Http/Method.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"
#include "Meta/Functor.h"

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
    Sync.FireAndForget([&cp, &callback](const FRemotingServer& srv) NOEXCEPT {
        callback(srv);
        cp.OnJobComplete(); // resume remoting job on worker thread
    });
    ITaskContext::Get().WaitFor(cp);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRemotingServer::FRemotingServer() NOEXCEPT
:   FHttpServer("Remoting", Network::FAddress::Localhost(1985))
{}
//----------------------------------------------------------------------------
FRemotingServer::~FRemotingServer() NOEXCEPT {
    Assert_NoAssume(not _sync); // should have already flush all callbacks :/
}
//----------------------------------------------------------------------------
void FRemotingServer::Add(URemotingEndpoint&& endpoint) {
    Assert(endpoint);

    Network::FName prefix{ endpoint->EndpointPrefix() };
    Assert(not prefix.empty());

    _endpoints.LockExclusive()->Insert_AssertUnique(std::move(prefix), std::move(endpoint));
}
//----------------------------------------------------------------------------
void FRemotingServer::Add(PBaseEndpoint&& endpoint) {
    Assert(endpoint);

    Network::FName prefix{ endpoint->EndpointPrefix() };
    Assert(not prefix.empty());

    if (not endpoint->RTTI_IsLoaded())
        endpoint->RTTI_EndpointAutomaticBinding();

    _endpoints.LockExclusive()->Insert_AssertUnique(std::move(prefix), std::move(endpoint));
}
//----------------------------------------------------------------------------
void FRemotingServer::Remove(const PBaseEndpoint& endpoint) {
    Assert(endpoint);

    const Network::FName prefix{ endpoint->EndpointPrefix() };

    const auto exclusiveEndpoints = _endpoints.LockExclusive();
    const auto it = exclusiveEndpoints->find(prefix);
    AssertReleaseMessage(L"endpoint not registered", exclusiveEndpoints->end() != it);
    Assert_NoAssume(std::get<PBaseEndpoint>(it->second) == endpoint);

    exclusiveEndpoints->Erase(it);
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

    _sync.FireAndForget(*this);

    Network::FHttpServer::Shutdown();
}
//----------------------------------------------------------------------------
bool FRemotingServer::OnRequest(Network::FServicingPort& port, const FRemotingRequest& request) const {
    static constexpr auto PathSeparator = Network::FUri::PathSeparator;

    const auto sharedEndpoints = _endpoints.LockShared(); // maintain a shared lock during a request

    IRemotingEndpoint* pEndpoint = nullptr;

    const FStringView& path = request.Uri().Path();
    Assert_NoAssume(PathSeparator == path.front());
    auto sep = path.FindAfter(PathSeparator, path.begin()/* skip first */);

    const FStringView name = path.CutBefore(sep);
    if (not name.empty()) {
        auto it = sharedEndpoints->find(Network::FName{ name });
        if (sharedEndpoints->end() != it) {
            Meta::Visit(it->second,
                [&pEndpoint](const URemotingEndpoint& uniq) NOEXCEPT {
                    pEndpoint = uniq.get();
                },
                [&pEndpoint](const PBaseEndpoint& shared) NOEXCEPT {
                    pEndpoint = shared.get();
                });
            Assert(pEndpoint);
        }
    }

    FRemotingResponse response;
    response.HTTP_SetServer(MakeStringView("PPE-remoting-server"));
    response.HTTP_SetAccessControlAllowOrigin(MakeStringView("*")); // avoid CORS policy issues

    if (Likely(pEndpoint)) {
        response.SetStatus(Network::EHttpStatus::OK);

        Network::FHttpHeader::FCookieMap cookie;
        if (not Network::FHttpHeader::UnpackCookie(&cookie, request) )
            cookie.clear_ReleaseMemory(); // no cookie present

        Network::FHttpHeader::FPostMap post;
        if (request.Method() == Network::EHttpMethod::Post &&
            not Network::FHttpHeader::UnpackPost(&post, request) ) {
            LOG(Remoting, Warning, L"failed to unpack post parameters from request: {0}", request.Uri());
            post.clear_ReleaseMemory();
        }

        Network::FUriQueryMap query;
        if (not Network::FUri::Unpack(query, request.Uri())) {
            LOG(Remoting, Warning, L"failed to unpack query parameters from uri: {0}", request.Uri());
            query.clear_ReleaseMemory();
        }

        pEndpoint->EndpointProcess(FRemotingContext{
            &response,
            Localhost(),
            IterateOnEndpoints(*sharedEndpoints),
            request,
            cookie,
            post,
            query,
            _sync.Public() });
    }
    else {
        response.SetStatus(Network::EHttpStatus::Forbidden);
        response.SetReason("No endpoint found");
    }

    return (FRemotingResponse::Write(&port.Socket(), response) &&
            request.AskToKeepAlive() );
}
//----------------------------------------------------------------------------
const IRemotingEndpoint& FRemotingServer::EndpointVisit(const FRegisteredEndpoint& variant) NOEXCEPT {
    return *Meta::Visit(variant, [](const auto& ptrLike) NOEXCEPT -> const IRemotingEndpoint* {
        return ptrLike.get();
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
