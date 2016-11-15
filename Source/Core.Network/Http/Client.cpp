#include "stdafx.h"

#include "Client.h"

#include "ConstNames.h"
#include "Exceptions.h"
#include "Method.h"
#include "Request.h"
#include "Response.h"
#include "Status.h"

#include "../Socket/Address.h"
#include "../Uri.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void HttpTypicalRequestHeaders_(FHttpRequest* request) {
    request->Add(FHttpConstNames::UserAgent(),      "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5 (.NET CLR 3.5.30729)");
    request->Add(FHttpConstNames::Accept(),         "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request->Add(FHttpConstNames::AcceptLanguage(), "en-us,en;q=0.5");
    request->Add(FHttpConstNames::AcceptEncoding(), "identity");
    request->Add(FHttpConstNames::AcceptCharset(),  "utf-8");
    request->Add(FHttpConstNames::KeepAlive(),      "300");
    request->Add(FHttpConstNames::CacheControl(),   "no-cache");
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
    prequest->Add(FHttpConstNames::Host(), ToString(hostname));

    HttpTypicalRequestHeaders_(prequest);

    if (cookie.size())
        FHttpRequest::PackCookie(prequest, cookie);
}
//----------------------------------------------------------------------------
template <typename _Method>
static bool SafeHttpClient_(const FUri& uri, const _Method& method) {
    Assert(uri.IsAbsolute());
    CORE_TRY {
        FHttpClient cli(uri.Hostname(), FHttpClient::DefaultPort);
        method(uri, cli);
    }
    CORE_CATCH(FHttpException e)
    CORE_CATCH_BLOCK({
        LOG(Error, L"[HTTP] {0}: {1}, {2}",  uri.Str(), e.Status(), e.what());
        return false;
    })
    return true;
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
        CORE_THROW_IT(FHttpException(EHttpStatus::ServiceUnavailable, "Failed to open connection"));
}
//----------------------------------------------------------------------------
FHttpClient::FHttpClient(const FAddress& address, size_t maxContentLength/* = DefaultMaxContentLength */)
:   FHttpClient(FAddress(address), maxContentLength) {}
//----------------------------------------------------------------------------
FHttpClient::FHttpClient(const FStringView& hostname, size_t port/* = DefaultPort */, size_t maxContentLength/* = DefaultMaxContentLength */)
:   FHttpClient(FAddress(hostname, port), maxContentLength) {}
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
bool HttpGet(FHttpResponse* presponse, const FUri& uri) {
    return SafeHttpClient_(uri, [=](const FUri& uri, FHttpClient& cli) {
        cli.Get(presponse, uri);
    });
}
//----------------------------------------------------------------------------
bool HttpHead(FHttpResponse* presponse, const FUri& uri) {
    return SafeHttpClient_(uri, [=](const FUri& uri, FHttpClient& cli) {
        cli.Head(presponse, uri);
    });
}
//----------------------------------------------------------------------------
bool HttpPost(FHttpResponse* presponse, const FUri& uri, const FHttpClient::FPostMap& post) {
    return SafeHttpClient_(uri, [presponse, &post](const FUri& uri, FHttpClient& cli) {
        cli.Post(presponse, uri, post);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
