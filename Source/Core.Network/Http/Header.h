#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Name.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"
#include "Core/Memory/MemoryStream.h"

namespace Core {
namespace Network {
enum class EHttpMethod;
enum class EHttpStatus;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpHeader {
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
} //!namespace Core
