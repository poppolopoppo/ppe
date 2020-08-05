#include "stdafx.h"

#include "Http/Client.h"

#include "Http/ConstNames.h"
#include "Http/Exceptions.h"
#include "Http/Method.h"
#include "Http/Request.h"
#include "Http/Response.h"
#include "Http/Status.h"

#include "Socket/Address.h"
#include "Uri.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace Network {
EXTERN_LOG_CATEGORY(PPE_NETWORK_API,Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void HttpTypicalRequestHeaders_(FHttpRequest* request) {
    request->Add(FHttpHeaders::UserAgent(),      FString("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:61.0) Gecko/20100101 Firefox/61.0"));
    request->Add(FHttpHeaders::Accept(),         FString("text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"));
    request->Add(FHttpHeaders::AcceptLanguage(), FString("en-us,en;q=0.5"));
    request->Add(FHttpHeaders::AcceptEncoding(), FString("identity"));
    request->Add(FHttpHeaders::Connection(),     FString("keep-alive"));
    request->Add(FHttpHeaders::CacheControl(),   FString("max-age=0"));
}
//----------------------------------------------------------------------------
static void HttpMakeRequest_(
    FHttpRequest* prequest,
    EHttpMethod method,
    const FStringView& hostname,
    const FUri& uri,
    const FHttpClient::FCookieMap& cookie ) {
    prequest->SetMethod(method);
    prequest->SetUri(FUri(uri));
    prequest->Add(FHttpHeaders::Host(), ToString(hostname));

    HttpTypicalRequestHeaders_(prequest);

    if (cookie.size())
        FHttpRequest::PackCookie(prequest, cookie);
}
//----------------------------------------------------------------------------
template <typename _Method>
static EHttpStatus SafeHttpClient_(const FUri& uri, const _Method& method) {
    Assert(uri.IsAbsolute());
    PPE_TRY {
        FHttpClient cli(uri.Hostname(), EServiceName::HTTP);
        method(uri, cli);
    }
    PPE_CATCH(FHttpException e)
    PPE_CATCH_BLOCK({
        LOG(Network, Error, L"HTTP {0}: {1}, {2}",  uri.Str(), e.Status(), MakeCStringView(e.What()));
        return e.Status();
    })
    return EHttpStatus::OK;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHttpClient::FHttpClient(FAddress&& address, size_t maxContentLength/* = DefaultMaxContentLength */)
:   _address(std::move(address))
,   _maxContentLength(maxContentLength) {
    Assert(not _address.empty());
    Assert(_maxContentLength > 0);

    if (not FSocketBuffered::MakeConnection(_socket, _address))
        PPE_THROW_IT(FHttpException(EHttpStatus::ServiceUnavailable, "failed to open connection"));
}
//----------------------------------------------------------------------------
FHttpClient::FHttpClient(const FAddress& address, size_t maxContentLength/* = DefaultMaxContentLength */)
:   FHttpClient(FAddress(address), maxContentLength) {}
//----------------------------------------------------------------------------
FHttpClient::FHttpClient(const FStringView& hostname, size_t port/* = size_t(EServiceName::HTTP) */, size_t maxContentLength/* = DefaultMaxContentLength */)
:   FHttpClient(FAddress(hostname, port), maxContentLength) {}
//----------------------------------------------------------------------------
FHttpClient::FHttpClient(const FStringView& hostname, EServiceName service/* = size_t(EServiceName::HTTP) */, size_t maxContentLength/* = DefaultMaxContentLength */)
    : FHttpClient(FAddress(hostname, service), maxContentLength) {}
//----------------------------------------------------------------------------
FHttpClient::~FHttpClient() {
    if (_socket.IsConnected())
        _socket.Disconnect();
}
//----------------------------------------------------------------------------
void FHttpClient::Get(FHttpResponse* presponse, const FUri& uri) {
    FHttpRequest request;
    HttpMakeRequest_(&request, EHttpMethod::Get, _address.Host(), uri, _cookie);

    Process(presponse, request);
}
//----------------------------------------------------------------------------
void FHttpClient::Head(FHttpResponse* presponse, const FUri& uri) {
    FHttpRequest request;
    HttpMakeRequest_(&request, EHttpMethod::Head, _address.Host(), uri, _cookie);

    Process(presponse, request);
}
//----------------------------------------------------------------------------
void FHttpClient::Post(FHttpResponse* presponse, const FUri& uri, const FPostMap& post) {
    FHttpRequest request;
    HttpMakeRequest_(&request, EHttpMethod::Post, _address.Host(), uri, _cookie);

    FHttpHeader::PackPost(&request, post);

    Process(presponse, request);
}
//----------------------------------------------------------------------------
void FHttpClient::Process(FHttpResponse* presponse, const FHttpRequest& request) {
    Assert(presponse);
    Assert(_socket.IsConnected());

    FHttpRequest::Write(&_socket, request);
    FHttpResponse::Read(presponse, _socket, _maxContentLength);

    FHttpHeader::UnpackCookie(&_cookie, *presponse);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EHttpStatus HttpGet(FHttpResponse* presponse, const FUri& uri) {
    return SafeHttpClient_(uri, [=](const FUri& uri, FHttpClient& cli) {
        cli.Get(presponse, uri);
    });
}
//----------------------------------------------------------------------------
EHttpStatus HttpHead(FHttpResponse* presponse, const FUri& uri) {
    return SafeHttpClient_(uri, [=](const FUri& uri, FHttpClient& cli) {
        cli.Head(presponse, uri);
    });
}
//----------------------------------------------------------------------------
EHttpStatus HttpPost(FHttpResponse* presponse, const FUri& uri, const FHttpClient::FPostMap& post) {
    return SafeHttpClient_(uri, [presponse, &post](const FUri& uri, FHttpClient& cli) {
        cli.Post(presponse, uri, post);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
