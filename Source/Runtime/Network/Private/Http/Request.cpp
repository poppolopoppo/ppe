// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Http/Request.h"

#include "Http/ConstNames.h"
#include "Http/Exceptions.h"
#include "Http/Method.h"
#include "Http/Status.h"

#include "Socket/SocketBuffered.h"
#include "Uri.h"

#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace Network {
EXTERN_LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void RequestReadUntil_(FTextWriter* poss, FSocketBuffered& socket, const char delim = '\n') {
    if (not socket.ReadUntil(poss, delim))
        PPE_THROW_IT(FHttpException(EHttpStatus::RequestURITooLong, "HTTP field from client terminated incorrectly"));

    socket.EatWhiteSpaces();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHttpRequest::FHttpRequest() : _method(EHttpMethod::Get) {}
//----------------------------------------------------------------------------
FHttpRequest::~FHttpRequest() = default;
//----------------------------------------------------------------------------
FHttpRequest::FHttpRequest(EHttpMethod method, FUri&& uri, FBody&& body)
:   FHttpHeader(std::move(body))
,   _method(method)
,   _uri(std::move(uri)) {}
//----------------------------------------------------------------------------
bool FHttpRequest::AskToKeepAlive() const NOEXCEPT {
    const FStringView connection = HTTP_Connection();
    if (EqualsI(connection, "close"_view))
        return false;
    if (EqualsI(connection, "keep-alive"_view))
        return true;
    //  Having a persistent connection is the default on HTTP/1.1 requests
    return true; // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Connection
}
//----------------------------------------------------------------------------
bool FHttpRequest::Read(FHttpRequest* prequest, FSocketBuffered& socket, size_t maxContentLength) {
    Assert(prequest);
    Assert(socket.IsConnected());

    if (not socket.IsReadable())
        return false;

    STACKLOCAL_TEXTWRITER(oss, 1024);

    prequest->Clear();

    // request type
    {
        RequestReadUntil_(&oss, socket, ' ');

        FStringView requestMethod = oss.Written();
        if (not HttpMethodFromCStr(&prequest->_method, requestMethod))
            PPE_THROW_IT(FHttpException(EHttpStatus::MethodNotAllowed, "HTTP invalid method requested"));

        oss.Reset();
    }

    // request path
    {
        RequestReadUntil_(&oss, socket, ' ');

        if (not FUri::Parse(prequest->_uri, oss.Written()))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP failed to parse requested path"));

        Assert_NoAssume(prequest->_uri.IsRelative());
        oss.Reset();
    }

    // protocol version
    {
        RequestReadUntil_(&oss, socket);

        const FStringView protocol = Strip(oss.Written());
        if (not EqualsI(protocol, FHttpHeader::ProtocolVersion().MakeView()) )
            PPE_THROW_IT(FHttpException(EHttpStatus::HTTPVersionNotSupported, "HTTP invalid protocol version, expected HTTP/1.1"));

        oss.Reset();
    }

    // headers
    {
        if (not FHttpHeader::Read(prequest, socket))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP malformed header field"));
    }

    // body
    {
        const FStringView contentLengthCStr = prequest->GetIFP(FHttpHeaders::ContentLength());

        if (contentLengthCStr.size()) {
            i64 contentLengthI = 0;
            if (not Atoi(&contentLengthI, contentLengthCStr, 10))
                PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP invalid content length"));

            const size_t contentLength = checked_cast<size_t>(contentLengthI);
            if (contentLength > maxContentLength)
                PPE_THROW_IT(FHttpException(EHttpStatus::RequestEntityTooLarge, "HTTP content length is too large"));

            const TMemoryView<u8> read = prequest->Body().Append(contentLength);
            if (contentLength != socket.Read(read) )
                PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP failed to read all content"));
        }

        oss.Reset();
    }

    return true;
}
//----------------------------------------------------------------------------
bool FHttpRequest::Write(FSocketBuffered* psocket, const FHttpRequest& request) {
    Assert(psocket);
    if (not psocket->IsConnected())
        return false;

    // method :
    PPE_LOG_CHECK(Network, psocket->Write(HttpMethodToCStr(request._method)));
    PPE_LOG_CHECK(Network, psocket->Put(' '));

    // path :
    if (request._uri.Path().size()) {
        PPE_LOG_CHECK(Network, psocket->Write(request._uri.Path()));
    }
    else {
        PPE_LOG_CHECK(Network, psocket->Put('/'));
    }
    if (request._uri.Query().size()) {
        PPE_LOG_CHECK(Network, psocket->Put('?'));
        PPE_LOG_CHECK(Network, psocket->Write(request._uri.Query()));
    }
    PPE_LOG_CHECK(Network, psocket->Put(' '));

    // protocol :
    PPE_LOG_CHECK(Network, psocket->Write(FHttpHeader::ProtocolVersion()));
    PPE_LOG_CHECK(Network, psocket->Write("\r\n"));

    // headers :
    for (const auto& it : request.Headers()) {
        PPE_LOG_CHECK(Network, psocket->Write(it.first.MakeView()));
        PPE_LOG_CHECK(Network, psocket->Write(": "));
        PPE_LOG_CHECK(Network, psocket->Write(it.second.MakeView()));
        PPE_LOG_CHECK(Network, psocket->Write("\r\n"));
    }

    // add content-length header if omitted :
    if (request.Body().SizeInBytes() &&
        request.GetIFP(FHttpHeaders::ContentLength()).empty()) {
        char tmp[32];
        FFixedSizeTextWriter oss(tmp);
        Format(oss, "{0}", request.Body().SizeInBytes());

        PPE_LOG_CHECK(Network, psocket->Write(FHttpHeaders::ContentLength().MakeView()));
        PPE_LOG_CHECK(Network, psocket->Write(": "));
        PPE_LOG_CHECK(Network, psocket->Write(oss.Written()));
        PPE_LOG_CHECK(Network, psocket->Write("\r\n"));
    }

    PPE_LOG_CHECK(Network, psocket->Write("\r\n"));

    // body :
    if (not request.Body().empty())
        PPE_LOG_CHECK(Network, psocket->Write(request.Body().MakeView()));

    return psocket->FlushWrite();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
