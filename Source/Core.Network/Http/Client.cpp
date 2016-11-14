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
static void HttpTypicalHeaders_(FHttpRequest* request) {
    request->Add(FHttpConstNames::UserAgent(),      "Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5 (.NET CLR 3.5.30729)");
    request->Add(FHttpConstNames::Accept(),         "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request->Add(FHttpConstNames::AcceptLanguage(), "en-us,en;q=0.5");
    request->Add(FHttpConstNames::AcceptEncoding(), "identity");
    request->Add(FHttpConstNames::AcceptCharset(),  "utf-8");
    request->Add(FHttpConstNames::KeepAlive(),      "300");
    request->Add(FHttpConstNames::CacheControl(),   "no-cache");
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

    request.SetMethod(EHttpMethod::Get);
    request.SetUri(FUri(uri));
    request.Add(FHttpConstNames::Host(), FString(_address.Host()) );

    HttpTypicalHeaders_(&request);

    Process(presponse, request);
}
//----------------------------------------------------------------------------
void FHttpClient::Head(FHttpResponse* presponse, const FUri& uri) {
    FHttpRequest request;

    request.SetMethod(EHttpMethod::Head);
    request.SetUri(FUri(uri));
    request.Add(FHttpConstNames::Host(), FString(_address.Host()) );

    HttpTypicalHeaders_(&request);

    Process(presponse, request);
}
//----------------------------------------------------------------------------
void FHttpClient::Process(FHttpResponse* presponse, const FHttpRequest& request) {
    Assert(presponse);
    Assert(_socket.IsConnected());

    FHttpRequest::Write(&_socket, request);
    FHttpResponse::Read(presponse, _socket, _maxContentLength);
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
