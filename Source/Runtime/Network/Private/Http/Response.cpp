#include "stdafx.h"

#include "Response.h"

#include "ConstNames.h"
#include "Exceptions.h"
#include "Status.h"

#include "../Socket/SocketBuffered.h"
#include "../Uri.h"

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
FHttpResponse::~FHttpResponse() {}
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
void FHttpResponse::UpdateContentHeaders(const FStringView& mimeType) {
    FString contentLength;
    Format(contentLength, "{0}", Body().SizeInBytes());

    Add(FHttpConstNames::ContentType(), ToString(mimeType));
    Add(FHttpConstNames::ContentLength(), std::move(contentLength));
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

        i32 statusCodeI = 0;
        const FStringView statusCodeCstr = Strip(oss.Written());
        if (not Atoi(&statusCodeI, statusCodeCstr, 10))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP invalid status code"));

        presponse->_status = EHttpStatus(statusCodeI);

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
        const FStringView contentLengthCStr = presponse->GetIFP(FHttpConstNames::ContentLength());

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
void FHttpResponse::Write(FSocketBuffered* psocket, const FHttpResponse& response) {
    Assert(psocket);
    Assert(psocket->IsConnected());

    psocket->Write(FHttpHeader::ProtocolVersion());
    psocket->Put(' ');
    psocket->Write(HttpStatusCode(response._status));
    psocket->Put(' ');
    psocket->Write(HttpStatusName(response._status));
    if (response._reason.size()) {
        psocket->Put(' ');
        psocket->Write(MakeStringView(response._reason));
    }
    psocket->Write("\r\n");

    for (const auto& it : response.Headers()) {
        psocket->Write(it.first.MakeView());
        psocket->Write(": ");
        psocket->Write(it.second.MakeView());
        psocket->Write("\r\n");
    }

    psocket->Write("\r\n");

    if (not response.Body().empty())
        psocket->Write(response.Body().MakeView());

    psocket->FlushWrite();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
