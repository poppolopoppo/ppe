#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Socket/Address.h"
#include "Core.Network/Socket/SocketBuffered.h"

namespace Core {
namespace Network {
class FUri;
class FHttpRequest;
class FHttpResponse;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpClient {
public:
    STATIC_CONST_INTEGRAL(size_t, DefaultPort, 80);
    STATIC_CONST_INTEGRAL(size_t, DefaultMaxContentLength, 10*1024*1024);

    explicit FHttpClient(FAddress&& address, size_t maxContentLength = DefaultMaxContentLength);
    explicit FHttpClient(const FAddress& address, size_t maxContentLength = DefaultMaxContentLength);
    FHttpClient(const FStringView& hostname, size_t port = DefaultPort, size_t maxContentLength = DefaultMaxContentLength);
    ~FHttpClient();

    FHttpClient(const FHttpClient& ) = delete;
    FHttpClient& operator =(const FHttpClient& ) = delete;

    const FAddress& Address() const { return _address; }
    size_t MaxContentLength() const { return _maxContentLength; }

    void Get(FHttpResponse* presponse, const FUri& uri);
    void Head(FHttpResponse* presponse, const FUri& uri);

    void Process(FHttpResponse* presponse, const FHttpRequest& request);

private:
    FAddress _address;
    size_t _maxContentLength;
    FSocketBuffered _socket;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool HttpGet(FHttpResponse* presponse, const FUri& uri);
//----------------------------------------------------------------------------
bool HttpHead(FHttpResponse* presponse, const FUri& uri);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
