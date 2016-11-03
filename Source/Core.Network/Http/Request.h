#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Http/Header.h"
#include "Core.Network/Uri.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/RawStorage.h"

namespace Core {
namespace Network {
enum class EHttpMethod;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpRequest : public FHttpHeader {
public:
    typedef RAWSTORAGE(HTTP, char) FBody;
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FString, FString, 3) FCookieMap;

    FHttpRequest();
    ~FHttpRequest();

    FHttpRequest(EHttpMethod method, FUri&& uri, FBody&& body);

    EHttpMethod Method() const { return _method; }
    void SetMethod(EHttpMethod method) { _method = method; }

    const FUri& Uri() const { return _uri; }
    void SetUri(FUri&& uri) { _uri = std::move(uri); }

    FStringView Body() const { return _body.MakeConstView(); }
    void SetBody(FBody&& body) { _body = std::move(body); }

    static void Read(FHttpRequest* prequest, FSocketBuffered& socket, size_t maxContentLength);
    static bool UnpackCookie(FCookieMap* pcookie, FHttpRequest& request);

private:
    EHttpMethod _method;
    FUri _uri;
    RAWSTORAGE(HTTP, char) _body;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
