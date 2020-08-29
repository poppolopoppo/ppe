#pragma once

#include "Remoting_fwd.h"

#include "ApplicationBase.h"
#include "Http/Server.h"

#include "Container/AssociativeVector.h"
#include "Memory/UniquePtr.h"
#include "Misc/Event.h"
#include "Misc/Function.h"
#include "Thread/ReadWriteLock.h"

namespace PPE {
namespace Remoting {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(RemotingEndpoint);
//----------------------------------------------------------------------------
struct FRemotingContext {
    FRemotingResponse* pResponse;
    const FRemotingRequest& Request;
    TPublicEvent<FRemotingCallback, true>& Sync;

    void Failed(Network::EHttpStatus status, FString&& reason) const;
    void BadRequest(const FStringView& what) const;
    void ExpectationFailed(const FStringView& what) const;
    void NotFound(const FStringView& what) const;

    void WaitForSync(FRemotingCallback&& callback) const NOEXCEPT;
};
//----------------------------------------------------------------------------
class PPE_REMOTING_API FRemotingServer final : private Network::FHttpServer {
public:
    explicit FRemotingServer() NOEXCEPT;
    ~FRemotingServer() NOEXCEPT;

    void Add(URemotingEndpoint&& endpoint);

    void Start();
    void Tick(FTimespan dt);
    void Shutdown();

private:
    virtual bool OnRequest(Network::FServicingPort& port, const FRemotingRequest& request) const override;

    mutable FReadWriteLock _barrierRW;
    TAssociativeVector<
        FStringView, URemotingEndpoint,
        TStringViewEqualTo<char, ECase::Insensitive>,
        VECTORINSITU(Remoting, TPair<FStringView COMMA URemotingEndpoint>, 8) > _endpoints;

    mutable TThreadSafeEvent<FRemotingCallback> _sync;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Remoting
} //!namespace PPE
