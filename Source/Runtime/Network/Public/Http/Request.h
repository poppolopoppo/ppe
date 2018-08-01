#pragma once

#include "Network.h"

#include "Http/Header.h"
#include "Uri.h"

#include "Container/AssociativeVector.h"
#include "Memory/MemoryStream.h"

namespace PPE {
namespace Network {
enum class EHttpMethod;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpRequest : public FHttpHeader {
public:
    FHttpRequest();
    ~FHttpRequest();

    FHttpRequest(EHttpMethod method, FUri&& uri, FBody&& body);

    EHttpMethod Method() const { return _method; }
    void SetMethod(EHttpMethod method) { _method = method; }

    const FUri& Uri() const { return _uri; }
    void SetUri(FUri&& uri) { _uri = std::move(uri); }

    static void Read(FHttpRequest* prequest, FSocketBuffered& socket, size_t maxContentLength);
    static void Write(FSocketBuffered* psocket, const FHttpRequest& request);

private:
    EHttpMethod _method;
    FUri _uri;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
