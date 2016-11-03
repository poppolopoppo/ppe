#include "stdafx.h"

#include "Response.h"

#include "ConstNames.h"
#include "Status.h"

#include "../Socket/SocketBuffered.h"
#include "../Uri.h"

namespace Core {
namespace Network {
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
    STACKLOCAL_OCSTRSTREAM(oss, 32);
    oss << _body.SizeInBytes();

    Add(FHttpConstNames::ContentType(), ToString(mimeType) );
    Add(FHttpConstNames::ContentLength(), ToString(oss.MakeView()) );
}
//----------------------------------------------------------------------------
void FHttpResponse::Write(FSocketBuffered* psocket, const FHttpResponse& response) {
    Assert(psocket);
    Assert(psocket->IsConnected());

    psocket->Write("HTTP/1.1 ");
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

    if (not response._body.empty())
        psocket->Write(response._body.MakeView());

    psocket->FlushWrite();
}
//----------------------------------------------------------------------------
bool FHttpResponse::PackCookie(FHttpResponse* presponse, const FCookieMap& cookie) {
    Assert(presponse);

    FOStringStream oss;

    bool first = true;

    for (const auto& it : cookie) {
        if (first)
            first = false;
        else
            oss << " ; ";

        const FStringView key = MakeStringView(it.first);
        const FStringView value = MakeStringView(it.second);

        if (not FUri::Encode(oss, key))
            return false;

        oss << '=';

        if (not FUri::Encode(oss, value))
            return false;
    }

    presponse->Add(FHttpConstNames::Cookie(), std::move(oss.str()) );

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
