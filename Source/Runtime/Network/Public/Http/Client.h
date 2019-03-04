#pragma once

#include "Network.h"

#include "Socket/Address.h"
#include "Socket/SocketBuffered.h"

#include "Container/AssociativeVector.h"

namespace PPE {
namespace Network {
class FUri;
class FHttpRequest;
class FHttpResponse;
enum class EHttpStatus;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FHttpClient {
public:
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FString, FString, 3) FCookieMap;
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FString, FString, 3) FPostMap;

    STATIC_CONST_INTEGRAL(size_t, DefaultMaxContentLength, 10*1024*1024);

    explicit FHttpClient(FAddress&& address, size_t maxContentLength = DefaultMaxContentLength);
    explicit FHttpClient(const FAddress& address, size_t maxContentLength = DefaultMaxContentLength);

    FHttpClient(const FStringView& hostname, size_t port = size_t(EServiceName::HTTP), size_t maxContentLength = DefaultMaxContentLength);
    FHttpClient(const FStringView& hostname, EServiceName service = EServiceName::HTTP, size_t maxContentLength = DefaultMaxContentLength);

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
PPE_NETWORK_API EHttpStatus HttpGet(FHttpResponse* presponse, const FUri& uri);
//----------------------------------------------------------------------------
PPE_NETWORK_API EHttpStatus HttpHead(FHttpResponse* presponse, const FUri& uri);
//----------------------------------------------------------------------------
PPE_NETWORK_API EHttpStatus HttpPost(FHttpResponse* presponse, const FUri& uri, const FHttpClient::FPostMap& post);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
