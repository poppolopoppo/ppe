#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Http/Header.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Memory/MemoryStream.h"

namespace Core {
namespace Network {
enum class EHttpStatus;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpResponse : public FHttpHeader {
public:
    typedef MEMORYSTREAM(HTTP) FBody;
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FString, FString, 3) FCookieMap;

    FHttpResponse();
    ~FHttpResponse();

    FHttpResponse(EHttpStatus status, FString&& reason);

    EHttpStatus Status() const { return _status; }
    void SetStatus(EHttpStatus status) { _status = status; }

    FStringView Reason() const { return MakeStringView(_reason); }
    void SetReason(FString&& reason) { Assert(reason.size()); _reason = std::move(reason); }

    bool Succeed() const;
    bool Failed() const { return (not Succeed()); }

    FBody& Body() { return _body; }
    FStringView Body() const { return _body.MakeView().Cast<const char>(); }
    void SetBody(FBody&& body) { _body = std::move(_body); }

    void UpdateContentHeaders(const FStringView& mimeType);

    static void Write(FSocketBuffered* psocket, const FHttpResponse& response);
    static bool PackCookie(FHttpResponse* presponse, const FCookieMap& cookie);

private:
    EHttpStatus _status;
    FString _reason;
    FBody _body;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
