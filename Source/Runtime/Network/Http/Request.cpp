#include "stdafx.h"

#include "Request.h"

#include "ConstNames.h"
#include "Exceptions.h"
#include "Method.h"
#include "Status.h"

#include "../Socket/SocketBuffered.h"
#include "../Uri.h"

#include "Core/IO/Format.h"
#include "Core/IO/TextWriter.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void RequestReadUntil_(FTextWriter* poss, FSocketBuffered& socket, const char delim = '\n') {
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
:   FHttpHeader(std::move(body))
,   _method(method)
,   _uri(std::move(uri)) {}
//----------------------------------------------------------------------------
void FHttpRequest::Read(FHttpRequest* prequest, FSocketBuffered& socket, size_t maxContentLength) {
    Assert(prequest);
    Assert(socket.IsConnected());

    STACKLOCAL_TEXTWRITER(oss, 1024);

    prequest->Clear();

    // request type
    {
        RequestReadUntil_(&oss, socket, ' ');

        FStringView requestMethod = oss.Written();
        if (not HttpMethodFromCStr(&prequest->_method, requestMethod))
            CORE_THROW_IT(FHttpException(EHttpStatus::MethodNotAllowed, "HTTP invalid method requested"));

        oss.Reset();
    }

    // request path
    {
        RequestReadUntil_(&oss, socket, ' ');

        if (not FUri::Parse(prequest->_uri, oss.Written()))
            CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP failed to parse requested path"));

        oss.Reset();
    }

    // protocol version
    {
        RequestReadUntil_(&oss, socket);

        const FStringView protocol = Strip(oss.Written());
        if (not EqualsI(protocol, FHttpHeader::ProtocolVersion()) )
            CORE_THROW_IT(FHttpException(EHttpStatus::HTTPVersionNotSupported, "HTTP invalid protocol version, expected HTTP/1.1"));

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
            if (not Atoi(&contentLengthI, contentLengthCStr, 10))
                CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP invalid content length"));

            const size_t contentLength = checked_cast<size_t>(contentLengthI);
            if (contentLength > maxContentLength)
                CORE_THROW_IT(FHttpException(EHttpStatus::RequestEntityTooLarge, "HTTP content length is too large"));

            const TMemoryView<u8> read = prequest->Body().Append(contentLength);
            if (contentLength != socket.Read(read) )
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

    // add content-length header if omitted :
    if (request.Body().SizeInBytes() &&
        request.GetIFP(FHttpConstNames::ContentLength()).empty()) {
        char tmp[32];
        FFixedSizeTextWriter oss(tmp);
        Format(oss, "{0}", request.Body().SizeInBytes());

        psocket->Write(FHttpConstNames::ContentLength().MakeView());
        psocket->Write(": ");
        psocket->Write(oss.Written());
        psocket->Write("\r\n");
    }

    psocket->Write("\r\n");

    // body :
    if (not request.Body().empty())
        psocket->Write(request.Body().MakeView());

    psocket->FlushWrite();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
