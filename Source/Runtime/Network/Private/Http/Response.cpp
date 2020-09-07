#include "stdafx.h"

#include "Http/Response.h"

#include "Http/ConstNames.h"
#include "Http/Exceptions.h"
#include "Http/Status.h"

#include "Socket/SocketBuffered.h"
#include "Uri.h"

#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void ResponseReadUntil_(FTextWriter* poss, FSocketBuffered& socket, const char delim = '\n') {
    if (not socket.ReadUntil(poss, delim))
        PPE_THROW_IT(FHttpException(EHttpStatus::RequestURITooLong, "HTTP field from client terminated incorrectly"));

    socket.EatWhiteSpaces();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHttpResponse::FHttpResponse() : _status(EHttpStatus::InternalServerError) {}
//----------------------------------------------------------------------------
FHttpResponse::~FHttpResponse() = default;
//----------------------------------------------------------------------------
FHttpResponse::FHttpResponse(EHttpStatus status, FString&& reason)
:   _status(status)
,   _reason(std::move(reason)) {
    Assert(_reason.size());
}
//----------------------------------------------------------------------------
bool FHttpResponse::Succeed() const {
    return HttpIsSuccessful(_status);
}
//----------------------------------------------------------------------------
void FHttpResponse::OverrideBody(UStreamReader&& overrideBody) {
    Assert(overrideBody);
    Assert(not _overrideBody);

    _overrideBody = std::move(overrideBody);
}
//----------------------------------------------------------------------------
void FHttpResponse::UpdateContentHeaders(const FStringView& mimeType) {
    Add(FHttpHeaders::ContentType(), ToString(mimeType));
    if (not _overrideBody)
        Add(FHttpHeaders::ContentLength(), ToString(Body().SizeInBytes()));
}
//----------------------------------------------------------------------------
void FHttpResponse::Read(FHttpResponse* presponse, FSocketBuffered& socket, size_t maxContentLength) {
    Assert(presponse);
    Assert(socket.IsConnected());

    STACKLOCAL_TEXTWRITER(oss, 1024);

    presponse->Clear();

    // protocol version
    {
        ResponseReadUntil_(&oss, socket, ' ');

        const FStringView protocol = Strip(oss.Written());
        if (not EqualsI(protocol, FHttpHeader::ProtocolVersion()) )
            PPE_THROW_IT(FHttpException(EHttpStatus::HTTPVersionNotSupported, "HTTP invalid protocol version, expected HTTP/1.1"));

        oss.Reset();
    }

    // status code
    {
        ResponseReadUntil_(&oss, socket, ' ');

        i32 statusCodeN = 0;
        const FStringView statusCodeCstr = Strip(oss.Written());
        if (not Atoi(&statusCodeN, statusCodeCstr, 10))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP invalid status code"));

        presponse->_status = EHttpStatus(statusCodeN);

        oss.Reset();
    }

    // status message
    {
        ResponseReadUntil_(&oss, socket);

        const FStringView statusMsgCstr = Strip(oss.Written());
        presponse->_reason = ToString(statusMsgCstr);

        oss.Reset();
    }

    // headers
    {
        if (not FHttpHeader::Read(presponse, socket))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP malformed header field"));
    }

    // body
    {
        const FStringView contentLengthCStr = presponse->GetIFP(FHttpHeaders::ContentLength());

        if (contentLengthCStr.size()) {
            i64 contentLengthI = 0;
            if (not Atoi(&contentLengthI, contentLengthCStr, 10))
                PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP invalid content length"));

            const size_t contentLength = checked_cast<size_t>(contentLengthI);
            if (contentLength > maxContentLength)
                PPE_THROW_IT(FHttpException(EHttpStatus::RequestEntityTooLarge, "HTTP content length is too large"));

            if (contentLength != socket.Read(presponse->Body().Append(contentLength)) )
                PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP failed to read all content"));
        }

        oss.Reset();
    }
}
//----------------------------------------------------------------------------
bool FHttpResponse::Write(FSocketBuffered* psocket, const FHttpResponse& response) {
    Assert(psocket);
    Assert(psocket->IsConnected());

    bool succeed = true;

    succeed &= psocket->Write(ProtocolVersion());
    succeed &= psocket->Put(' ');
    succeed &= psocket->Write(HttpStatusCode(response.Status()));
    succeed &= psocket->Put(' ');
    succeed &= psocket->Write(HttpStatusName(response.Status()));
    if (response.Reason().size()) {
        succeed &= psocket->Put(' ');
        succeed &= psocket->Write(response.Reason());
    }
    succeed &= psocket->Write("\r\n");

    for (const auto& it : response.Headers()) {
        succeed &= psocket->Write(it.first.MakeView());
        succeed &= psocket->Write(": ");
        succeed &= psocket->Write(it.second.MakeView());
        succeed &= psocket->Write("\r\n");
    }

    succeed &= psocket->Write("\r\n");

    if (Likely(not response._overrideBody)) {
        if (not response.Body().empty())
            succeed &= psocket->Write(response.Body().MakeView());
    }
    else {
        Assert(response.Body().empty());

        u8 buf[2048]; // don't use STACKLOCAL() since overrideBody may change the executing thread thanks to fibers

        for (;;) {
            const size_t read = response._overrideBody->ReadSome(buf, sizeof(buf[0]), lengthof(buf));
            if (read > 0) {
                Assert(read <= lengthof(buf));
                if (psocket->Write(FRawMemoryConst{ buf, read }) != read)
                    return false;
                if (not psocket->FlushWrite())
                    break;
            }
        }
    }

    succeed &= psocket->FlushWrite();

    return succeed;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
