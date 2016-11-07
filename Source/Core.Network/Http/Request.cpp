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
static void ReadUntil_(std::ostream* poss, FSocketBuffered& socket, const char delim = '\n') {
    constexpr size_t maxLength = 64 * 1024;

    char ch;
    for(size_t len = 0;
        socket.Peek(ch) && ch != delim && ch != '\n';
        ++len ) {
        if (len == maxLength)
            CORE_THROW_IT(FHttpException(EHttpStatus::RequestURITooLong, "HTTP field from client is too long"));

        if (not socket.Get(ch))
            AssertNotReached();

        poss->put(ch);
    }

    if (not socket.Peek(ch))
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
        ReadUntil_(&oss, socket, ' ');

        FStringView requestMethod = oss.MakeView();
        if (not HttpMethodFromCStr(&prequest->_method, requestMethod))
            CORE_THROW_IT(FHttpException(EHttpStatus::MethodNotAllowed, "HTTP invalid method requested"));

        oss.Reset();
    }

    // request path
    {
        ReadUntil_(&oss, socket, ' ');

        if (not FUri::Parse(prequest->_uri, oss.MakeView()))
            CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP failed to parse requested path"));

        oss.Reset();
    }

    // protocol version
    {
        ReadUntil_(&oss, socket);

        const FStringView protocol = Strip(oss.MakeView());
        if (not EqualsI(protocol, "HTTP/1.1"))
            throw FHttpException(EHttpStatus::HTTPVersionNotSupported, "HTTP invalid protocol version, expected HTTP/1.1");

        oss.Reset();
    }

    // headers
    {
        char ch;
        while (socket.Peek(ch)) {
            ReadUntil_(&oss, socket);

            const FStringView line = Strip(oss.MakeView());
            const auto doublePoint = line.Find(':');

            if (line.end() == doublePoint) {
                if (line.size())
                    CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP malformed header field"));

                break;
            }
            else {
                const FStringView key = Strip(line.CutBefore(doublePoint));
                const FStringView value = Strip(line.CutStartingAt(doublePoint+1));

                if (key.empty())
                    CORE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP failed to parse header key"));

                prequest->Add(FName(key), ToString(value));

                oss.Reset();
            }
        }

        oss.Reset();
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
