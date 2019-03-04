#pragma once

#include "Network.h"

#include "NetworkName.h"

#include "Container/AssociativeVector.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Memory/MemoryStream.h"

namespace PPE {
namespace Network {
enum class EHttpMethod;
enum class EHttpStatus;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FHttpHeader {
public:
    typedef MEMORYSTREAM(HTTP) FBody;
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FName, FString, 9/* according to Chrome */) FEntries;
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FString, FString, 3) FCookieMap;
    typedef ASSOCIATIVE_VECTORINSITU(HTTP, FString, FString, 3) FPostMap;

    FHttpHeader();
    explicit FHttpHeader(FBody&& body);
    ~FHttpHeader();

    FHttpHeader(const FHttpHeader& ) = delete;
    FHttpHeader& operator =(const FHttpHeader& ) = delete;

    const FEntries& Headers() const { return _headers; }

    FBody& Body() { return _body; }
    const FBody& Body() const { return _body; }
    void SetBody(FBody&& body) { _body = std::move(body); }

    void Add(const FName& key, FString&& value);
    void Remove(const FName& key);

    FStringView Get(const FName& key) const;
    FStringView GetIFP(const FName& key) const;

    FStringView operator[](const FName& key) const { return GetIFP(key); }

    void Clear();

    FStringView MakeView() const { return _body.MakeView().Cast<const char>(); }

    static bool Read(FHttpHeader* pheader, FSocketBuffered& socket);

    static void PackCookie(FHttpHeader* pheader, const FCookieMap& cookie);
    static bool UnpackCookie(FCookieMap* pcookie, const FHttpHeader& header);

    static void PackPost(FHttpHeader* pheader, const FPostMap& post);
    static bool UnpackPost(FPostMap* ppost, const FHttpHeader& header);

    static FStringView ProtocolVersion();

private:
    FEntries _headers;
    FBody _body;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
