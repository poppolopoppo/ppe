#pragma once

#include "Network.h"

#include "Socket/Address.h"

#include "IO/String.h"
#include "Maths/Units.h"

#include <atomic>
#include <thread>

namespace PPE {
namespace Network {
class FHttpRequest;
class FHttpResponse;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FHttpServer {
public:
    STATIC_CONST_INTEGRAL(size_t, DefaultMaxContentLength, 10*1024*1024); // 10 mo

    FHttpServer(const FStringView& name, FAddress&& localhost, const FMilliseconds& timeout = 3.0_s, size_t maxContentLength = DefaultMaxContentLength);
    virtual ~FHttpServer();

    FHttpServer(const FHttpServer& ) = delete;
    FHttpServer& operator =(const FHttpServer& ) = delete;

    const FString& Name() const { return _name; }
    const FAddress& Localhost() const { return _localhost; }
    const FMilliseconds& Timeout() const { return _timeout; }
    size_t MaxContentLength() const { return _maxContentLength; }

    void* UserData() const { return _userData; }
    void SetUserData(void* userData) { _userData = userData; }

    bool IsRunning() const;

    void Start();
    void Stop();

protected:
    friend class FHttpServerImpl;

    virtual void OnAccept(FSocketBuffered& socket) const = 0;
    virtual void OnRequest(FSocketBuffered& socket, const FHttpRequest& request) const PPE_THROW() = 0;
    virtual void OnDisconnect(FSocketBuffered& socket) const = 0;

private:
    FString _name;
    FAddress _localhost;
    FMilliseconds _timeout;
    size_t _maxContentLength;
    void* _userData;

    std::atomic_bool _quit;
    std::thread _servicing;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
