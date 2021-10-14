#pragma once

#include "Remoting_fwd.h"

#include "Application/ApplicationBase.h"
#include "Http/Server.h"
#include "NetworkName.h"
#include "Uri.h"

#include "Container/FlatMap.h"
#include "Memory/UniquePtr.h"
#include "Misc/Event.h"
#include "Misc/Function.h"
#include "Thread/ThreadSafe.h"

#include <variant>

#include "Container/Enumerable.h"
#include "Http/Header.h"
#include "Meta/Functor.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(RemotingEndpoint);
//----------------------------------------------------------------------------
using FRemotingCookieMap = Network::FHttpHeader::FCookieMap;
using FRemotingPostMap = Network::FHttpHeader::FPostMap;
using FRemotingQueryMap = Network::FUri::FQueryMap;
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRemotingServer final : private Network::FHttpServer {
public:
    explicit FRemotingServer() NOEXCEPT;
    ~FRemotingServer() NOEXCEPT override;

    FRemotingServer(const FRemotingServer& ) = delete;
    FRemotingServer& operator =(const FRemotingServer& ) = delete;

    void Add(URemotingEndpoint&& endpoint);

    void Add(PBaseEndpoint&& endpoint);
    void Remove(const PBaseEndpoint& endpoint);

    void Start();
    void Tick(FTimespan dt);
    void Shutdown();

private:
    friend struct FRemotingContext;

    using FRegisteredEndpoint = std::variant<
        URemotingEndpoint,
        PBaseEndpoint >;

    using FEndpointMap = FLATMAP_INSITU(Remoting, Network::FName, FRegisteredEndpoint, 8);

    static const IRemotingEndpoint& EndpointVisit(const FRegisteredEndpoint& variant) NOEXCEPT;
    static auto IterateOnEndpoints(const FEndpointMap& endpoints) NOEXCEPT {
        return endpoints.Values().Map(&FRemotingServer::EndpointVisit);
    }

    virtual bool OnRequest(Network::FServicingPort& port, const FRemotingRequest& request) const override;

    TThreadSafe<FEndpointMap, EThreadBarrier::RWLock> _endpoints;

    mutable TThreadSafeEvent<FRemotingCallback> _sync;
};
//----------------------------------------------------------------------------
struct FRemotingContext {
    using FEndpointEnumerator = decltype(FRemotingServer::IterateOnEndpoints(
        std::declval<const FRemotingServer::FEndpointMap&>() ));

    FRemotingResponse* pResponse;
    const Network::FAddress& Localhost;
    const FEndpointEnumerator Endpoints;
    const FRemotingRequest& Request;
    const FRemotingCookieMap& Cookie;
    const FRemotingPostMap& Post;
    const FRemotingQueryMap& Query;
    TPublicEvent<FRemotingCallback, true>& Sync;

    void Failed(Network::EHttpStatus status, FString&& reason) const;
    void BadRequest(const FStringView& what) const;
    void ExpectationFailed(const FStringView& what) const;
    void NotFound(const FStringView& what) const;

    void WaitForSync(FRemotingCallback&& callback) const NOEXCEPT;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
