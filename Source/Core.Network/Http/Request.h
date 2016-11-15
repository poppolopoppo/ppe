#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Http/Header.h"
#include "Core.Network/Uri.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Memory/MemoryStream.h"

namespace Core {
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
} //!namespace Core
