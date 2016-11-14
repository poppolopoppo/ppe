#include "stdafx.h"

#include "Request.h"

#include "ConstNames.h"
#include "Exceptions.h"
#include "Method.h"
#include "Status.h"

#include "../Socket/SocketBuffered.h"
#include "../Uri.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void RequestReadUntil_(std::ostream* poss, FSocketBuffered& socket, const char delim = '\n') {
    if (not socket.ReadUntil(poss, delim))
        CORE_THROW_IT(FHttpException(EHttpStatus::RequestURITooLong, "HTTP field from client terminated incorrectly"));

    socket.EatWhiteSpaces();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHttpRequest::FHttpRequest() : _method(EHttpMethod::Get) {}
//----------------------------------------------------------------------------
FHttpRequest::~FHttpRequest() {}
//----------------------------------------------------------------------------
FHttpRequest::FHttpRequest(EHttpMethod method, FUri&& uri, FBody&& body)
:   _method(method)
,   _uri(std::move(uri))
,   _body(std::move(body)) {}
//----------------------------------------------------------------------------
void FHttpRequest::Read(FHttpRequest* prequest, FSocketBuffered& socket, size_t maxContentLength) {
    Assert(prequest);
    Assert(socket.IsConnected());

    STACKLOCAL_OCSTRSTREAM(oss, 1024);

    prequest->Clear();

    // request type
    {
        RequestReadUntil_(&oss, socket, ' ');

        FStringView requestMethod = oss.MakeView();
        if (not HttpMethodFromCStr(&prequest->_method, requestMethod))
            CORE_THROW_IT(FHttpException(EHttpStatus::MethodNotAllowed, "HTTP invalid method requested"));

        oss.Reset();
    }

    // request path
    {
        RequestReadUntil_(&oss, socket, ' ');

        if (not FUri::Parse(prequest->_uri, oss.MakeView()))
            CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP failed to parse requested path"));

        oss.Reset();
    }

    // protocol version
    {
        RequestReadUntil_(&oss, socket);

        const FStringView protocol = Strip(oss.MakeView());
        if (not EqualsI(protocol, FHttpHeader::ProtocolVersion()) )
            throw FHttpException(EHttpStatus::HTTPVersionNotSupported, "HTTP invalid protocol version, expected HTTP/1.1");

        oss.Reset();
    }

    // headers
    {
        if (not FHttpHeader::Read(prequest, socket))
            CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP malformed header field"));
    }

    // body
    {
        const FStringView contentLengthCStr = prequest->GetIFP(FHttpConstNames::ContentLength());

        if (contentLengthCStr.size()) {
            i64 contentLengthI = 0;
            if (not Atoi64(&contentLengthI, contentLengthCStr, 10))
                CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP invalid content length"));

            const size_t contentLength = checked_cast<size_t>(contentLengthI);
            if (contentLength > maxContentLength)
                CORE_THROW_IT(FHttpException(EHttpStatus::RequestEntityTooLarge, "HTTP content length is too large"));

            prequest->_body.Resize_DiscardData(contentLength);
            if (contentLength != socket.Read(prequest->_body.MakeView().Cast<u8>()) )
                CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP failed to read all content"));
        }

        oss.Reset();
    }
}
//----------------------------------------------------------------------------
void FHttpRequest::Write(FSocketBuffered* psocket, const FHttpRequest& request) {
    Assert(psocket);
    Assert(psocket->IsConnected());

    // method :
    psocket->Write(HttpMethodToCStr(request._method));
    psocket->Put(' ');

    // path :
    if (request._uri.Path().size()) {
        psocket->Write(request._uri.Path());
    }
    else {
        psocket->Put('/');
    }
    if (request._uri.Query().size()) {
        psocket->Put('?');
        psocket->Write(request._uri.Query());
    }
    psocket->Put(' ');

    // protocol :
    psocket->Write(FHttpHeader::ProtocolVersion());
    psocket->Write("\r\n");

    // headers :
    for (const auto& it : request.Headers()) {
        psocket->Write(it.first.MakeView());
        psocket->Write(": ");
        psocket->Write(it.second.MakeView());
        psocket->Write("\r\n");
    }

    psocket->Write("\r\n");

    if (not request._body.empty())
        psocket->Write(FStringView(request._body.MakeConstView()));

    psocket->FlushWrite();
}
//----------------------------------------------------------------------------
bool FHttpRequest::UnpackCookie(FCookieMap* pcookie, FHttpRequest& request) {
    Assert(pcookie);

    pcookie->clear();

    FStringView cookieCStr = request.GetIFP(FHttpConstNames::Cookie());
    if (cookieCStr.empty())
        return false;

    FStringView cookieLine;
    while (Split(cookieCStr, ';', cookieLine)) {
        FStringView encodedKey;
        FStringView encodedValue = cookieLine;
        if (not Split(encodedValue, '=', encodedKey) || encodedKey.empty())
            CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP malformed cookie entry"));

        encodedKey = Strip(encodedKey);
        encodedValue = Strip(encodedValue);

        FString decodedKey;
        if (not FUri::Decode(decodedKey, encodedKey))
            CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP could not decode cookie key"));

        FString decodedValue;
        if (not FUri::Decode(decodedValue, encodedValue))
            CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP could not decode cookie value"));

        pcookie->Insert_AssertUnique(std::move(decodedKey), std::move(decodedValue));
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
