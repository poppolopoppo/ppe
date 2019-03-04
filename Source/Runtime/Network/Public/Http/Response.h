#pragma once

#include "Network.h"

#include "Http/Header.h"

namespace PPE {
namespace Network {
enum class EHttpStatus;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FHttpResponse : public FHttpHeader {
public:
    FHttpResponse();
    ~FHttpResponse();

    FHttpResponse(EHttpStatus status, FString&& reason);

    EHttpStatus Status() const { return _status; }
    void SetStatus(EHttpStatus status) { _status = status; }

    FStringView Reason() const { return MakeStringView(_reason); }
    void SetReason(FString&& reason) { Assert(reason.size()); _reason = std::move(reason); }

    bool Succeed() const;
    bool Failed() const { return (not Succeed()); }

    void UpdateContentHeaders(const FStringView& mimeType);

    static void Read(FHttpResponse* presponse, FSocketBuffered& socket, size_t maxContentLength);
    static void Write(FSocketBuffered* psocket, const FHttpResponse& response);

private:
    EHttpStatus _status;
    FString _reason;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
