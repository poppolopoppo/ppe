#pragma once

#include "Network_fwd.h"

#include "Socket/Address.h"
#include "Socket/ServicingPort.h"

#include "IO/String.h"
#include "Maths/Units.h"
#include "Misc/Guid.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FHttpServer : Meta::FNonCopyable {
public:
    FHttpServer(
        const FStringView& name,
        FAddress&& localhost,
        FMilliseconds timeout = 1.0_s,
        FBytes maxContentLength = 10.0_mb );
    virtual ~FHttpServer();

    const FString& Name() const { return _name; }
    const FAddress& Localhost() const { return _localhost; }

    size_t MaxContentLength() const { return _maxContentLength; }

    const FMilliseconds& Timeout() const { return _timeout; }
    void SetTimeout(FMilliseconds value) { _timeout = value; }

    bool IsRunning() const;

    void Start(size_t workerCount);
    void Shutdown();

protected:
    virtual void OnConnect(FServicingPort& port) const;
    virtual bool OnRequest(FServicingPort& port, const FHttpRequest& request) const;
    virtual void OnDisconnect(FServicingPort& port) const;

private:
    bool Servicing_ReturnKeepAlive_(FServicingPort& port) const;

    FString _name;
    FAddress _localhost;
    FMilliseconds _timeout;
    size_t _maxContentLength;

    PHandShaker _service;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
