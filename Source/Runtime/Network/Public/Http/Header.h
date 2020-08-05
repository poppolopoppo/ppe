#pragma once

#include "Network_fwd.h"

#include "NetworkName.h"

#include "Container/AssociativeVector.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "Memory/MemoryStream.h"

namespace PPE {
namespace Network {
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

public:
    FStringView HTTP_Accept() const NOEXCEPT;
    FStringView HTTP_AcceptCharset() const NOEXCEPT;
    FStringView HTTP_AcceptEncoding() const NOEXCEPT;
    FStringView HTTP_AcceptLanguage() const NOEXCEPT;
    FStringView HTTP_CacheControl() const NOEXCEPT;
    FStringView HTTP_Connection() const NOEXCEPT;
    FStringView HTTP_Cookie() const NOEXCEPT;
    FStringView HTTP_ContentLanguage() const NOEXCEPT;
    FStringView HTTP_ContentLength() const NOEXCEPT;
    FStringView HTTP_ContentType() const NOEXCEPT;
    FStringView HTTP_Date() const NOEXCEPT;
    FStringView HTTP_Host() const NOEXCEPT;
    FStringView HTTP_KeepAlive() const NOEXCEPT;
    FStringView HTTP_Location() const NOEXCEPT;
    FStringView HTTP_Referer() const NOEXCEPT;
    FStringView HTTP_RetryAfter() const NOEXCEPT;
    FStringView HTTP_Server() const NOEXCEPT;
    FStringView HTTP_Status() const NOEXCEPT;
    FStringView HTTP_UserAgent() const NOEXCEPT;

    void HTTP_SetAccept(FString&& value);
    void HTTP_SetAcceptCharset(FString&& value);
    void HTTP_SetAcceptEncoding(FString&& value);
    void HTTP_SetAcceptLanguage(FString&& value);
    void HTTP_SetCacheControl(FString&& value);
    void HTTP_SetConnection(FString&& value);
    void HTTP_SetCookie(FString&& value);
    void HTTP_SetContentLanguage(FString&& value);
    void HTTP_SetContentLength(FString&& value);
    void HTTP_SetContentType(FString&& value);
    void HTTP_SetDate(FString&& value);
    void HTTP_SetHost(FString&& value);
    void HTTP_SetKeepAlive(FString&& value);
    void HTTP_SetLocation(FString&& value);
    void HTTP_SetReferer(FString&& value);
    void HTTP_SetRetryAfter(FString&& value);
    void HTTP_SetServer(FString&& value);
    void HTTP_SetStatus(FString&& value);
    void HTTP_SetUserAgent(FString&& value);

    void HTTP_SetAccept(const FStringView& value);
    void HTTP_SetAcceptCharset(const FStringView& value);
    void HTTP_SetAcceptEncoding(const FStringView& value);
    void HTTP_SetAcceptLanguage(const FStringView& value);
    void HTTP_SetCacheControl(const FStringView& value);
    void HTTP_SetConnection(const FStringView& value);
    void HTTP_SetCookie(const FStringView& value);
    void HTTP_SetContentLanguage(const FStringView& value);
    void HTTP_SetContentLength(const FStringView& value);
    void HTTP_SetContentType(const FStringView& value);
    void HTTP_SetDate(const FStringView& value);
    void HTTP_SetHost(const FStringView& value);
    void HTTP_SetKeepAlive(const FStringView& value);
    void HTTP_SetLocation(const FStringView& value);
    void HTTP_SetReferer(const FStringView& value);
    void HTTP_SetRetryAfter(const FStringView& value);
    void HTTP_SetServer(const FStringView& value);
    void HTTP_SetStatus(const FStringView& value);
    void HTTP_SetUserAgent(const FStringView& value);

    void HTTP_SetAccept(const FName& value);
    void HTTP_SetAcceptCharset(const FName& value);
    void HTTP_SetAcceptEncoding(const FName& value);
    void HTTP_SetAcceptLanguage(const FName& value);
    void HTTP_SetCacheControl(const FName& value);
    void HTTP_SetConnection(const FName& value);
    void HTTP_SetCookie(const FName& value);
    void HTTP_SetContentLanguage(const FName& value);
    void HTTP_SetContentLength(const FName& value);
    void HTTP_SetContentType(const FName& value);
    void HTTP_SetDate(const FName& value);
    void HTTP_SetHost(const FName& value);
    void HTTP_SetKeepAlive(const FName& value);
    void HTTP_SetLocation(const FName& value);
    void HTTP_SetReferer(const FName& value);
    void HTTP_SetRetryAfter(const FName& value);
    void HTTP_SetServer(const FName& value);
    void HTTP_SetStatus(const FName& value);
    void HTTP_SetUserAgent(const FName& value);

private:
    FEntries _headers;
    FBody _body;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
