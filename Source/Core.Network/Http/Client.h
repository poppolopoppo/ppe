#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Socket/Address.h"
#include "Core.Network/Socket/SocketBuffered.h"

#include "Core/Container/AssociativeVector.h"

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
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FString, FString, 3) FCookieMap;
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FString, FString, 3) FPostMap;

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

    FCookieMap& Cookie() { return _cookie; }
    const FCookieMap& Cookie() const { return _cookie; }
    void SetCookie(FCookieMap&& cookie) { _cookie = std::move(cookie); }

    void Get(FHttpResponse* presponse, const FUri& uri);
    void Head(FHttpResponse* presponse, const FUri& uri);
    void Post(FHttpResponse* presponse, const FUri& uri, const FPostMap& post);

    void Process(FHttpResponse* presponse, const FHttpRequest& request);

private:
    FAddress _address;
    size_t _maxContentLength;
    FSocketBuffered _socket;
    FCookieMap _cookie;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool HttpGet(FHttpResponse* presponse, const FUri& uri);
//----------------------------------------------------------------------------
bool HttpHead(FHttpResponse* presponse, const FUri& uri);
//----------------------------------------------------------------------------
bool HttpPost(FHttpResponse* presponse, const FUri& uri, const FHttpClient::FPostMap& post);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
