#include "stdafx.h"

#include "Http/Header.h"

#include "Http/ConstNames.h"
#include "Http/Exceptions.h"
#include "Http/Status.h"

#include "Socket/SocketBuffered.h"
#include "Uri.h"

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
static bool HeaderReadUntil_(FTextWriter* poss, FSocketBuffered& socket, const char delim = '\n') {
    if (socket.ReadUntil(poss, delim)) {
        socket.EatWhiteSpaces();
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FHttpHeader::FHttpHeader() {}
//----------------------------------------------------------------------------
FHttpHeader::FHttpHeader(FBody&& body) : _body(std::move(body)) {}
//----------------------------------------------------------------------------
FHttpHeader::~FHttpHeader() = default;
//----------------------------------------------------------------------------
void FHttpHeader::Add(const FName& key, FString&& value) {
    Assert(not key.empty());
    Assert(not value.empty());

    _headers.GetOrAdd(key) = std::move(value);
}
//----------------------------------------------------------------------------
void FHttpHeader::Remove(const FName& key) {
    Assert(not key.empty());

    _headers.Remove_AssertExists(key);
}
//----------------------------------------------------------------------------
FStringView FHttpHeader::Get(const FName& key) const {
    Assert(not key.empty());

    return MakeStringView(_headers.At(key));
}
//----------------------------------------------------------------------------
FStringView FHttpHeader::GetIFP(const FName& key) const {
    Assert(not key.empty());

    const FString* pvalue = _headers.GetIFP(key);
    return (pvalue ? MakeStringView(*pvalue) : FStringView());
}
//----------------------------------------------------------------------------
void FHttpHeader::Clear() {
    _headers.clear();
    _body.clear();
}
//----------------------------------------------------------------------------
bool FHttpHeader::Read(FHttpHeader* pheader, FSocketBuffered& socket) {
    Assert(pheader);
    Assert(socket.IsConnected());

    STACKLOCAL_TEXTWRITER(oss, 1024);

    char ch;
    while (socket.Peek(ch)) {
        HeaderReadUntil_(&oss, socket);

        const FStringView line = Strip(oss.Written());
        const auto doublePoint = line.Find(':');

        if (line.end() == doublePoint) {
            if (line.size())
                return false;

            break;
        }
        else {
            const FStringView key = Strip(line.CutBefore(doublePoint));
            const FStringView value = Strip(line.CutStartingAt(doublePoint+1));

            if (key.empty())
                return false;

            pheader->Add(FName(key), ToString(value));

            oss.Reset();
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FHttpHeader::PackCookie(FHttpHeader* pheader, const FCookieMap& cookie) {
    Assert(pheader);

    FStringBuilder oss;

    bool many = false;
    for (const auto& it : cookie) {
        if (many) {
            oss.Put(';');
            oss.Put(' ');
        }

        if (not FUri::Encode(oss, it.first.MakeView()))
            return false;

        oss.Put('=');

        if (not FUri::Encode(oss, it.second.MakeView()))
            return false;

        many = true;
    }

    pheader->Add(FHttpHeaders::Cookie(), oss.ToString());
    return true;
}
//----------------------------------------------------------------------------
bool FHttpHeader::UnpackCookie(FCookieMap* pcookie, const FHttpHeader& header) {
    Assert(pcookie);

    pcookie->clear();

    FStringView cookieCStr = header.GetIFP(FHttpHeaders::Cookie());
    if (cookieCStr.empty())
        return false;

    FStringView cookieLine;
    while (Split(cookieCStr, ';', cookieLine)) {
        FStringView encodedKey;
        FStringView encodedValue = cookieLine;
        if (not Split(encodedValue, '=', encodedKey) || encodedKey.empty())
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP malformed cookie entry"));

        encodedKey = Strip(encodedKey);
        encodedValue = Strip(encodedValue);

        FString decodedKey;
        if (not FUri::Decode(decodedKey, encodedKey))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP could not decode cookie key"));

        FString decodedValue;
        if (not FUri::Decode(decodedValue, encodedValue))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP could not decode cookie value"));

        pcookie->Insert_AssertUnique(std::move(decodedKey), std::move(decodedValue));
    }

    return true;
}
//----------------------------------------------------------------------------
bool FHttpHeader::PackPost(FHttpHeader* pheader, const FPostMap& post) {
    Assert(pheader);

    pheader->_body.clear();

    FTextWriter oss(&pheader->_body);

    bool many = false;
    for (const auto& it : post) {
        if (many)
            oss.Put('&');

        if (not FUri::Encode(oss, it.first.MakeView()))
            return false;

        oss.Put('=');
        if (not FUri::Encode(oss, it.second.MakeView()))
            return false;

        many = true;
    }

    pheader->Add(FHttpHeaders::ContentType(), FString("application/x-www-form-urlencoded"));
    return true;
}
//----------------------------------------------------------------------------
bool FHttpHeader::UnpackPost(FPostMap* ppost, const FHttpHeader& header) {
    Assert(ppost);

    ppost->clear();

    FStringView body = header._body.MakeView().Cast<const char>();
    if (body.empty())
        return false;

    FStringView postkv;
    while (Split(body, '&', postkv)) {
        FStringView encodedKey;
        FStringView encodedValue = postkv;
        if (not Split(encodedValue, '=', encodedKey) || encodedKey.empty())
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP malformed POST entry"));

        encodedKey = Strip(encodedKey);
        encodedValue = Strip(encodedValue);

        FString decodedKey;
        if (not FUri::Decode(decodedKey, encodedKey))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP could not decode POST key"));

        FString decodedValue;
        if (not FUri::Decode(decodedValue, encodedValue))
            PPE_THROW_IT(FHttpException(EHttpStatus::BadRequest, "HTTP could not decode POST value"));

        ppost->Insert_AssertUnique(std::move(decodedKey), std::move(decodedValue));
    }

    return true;
}
//----------------------------------------------------------------------------
FStringView FHttpHeader::ProtocolVersion() {
    return "HTTP/1.1";
}
//----------------------------------------------------------------------------
FStringView FHttpHeader::HTTP_Accept() const NOEXCEPT { return GetIFP(FHttpHeaders::Accept()); }
FStringView FHttpHeader::HTTP_AcceptCharset() const NOEXCEPT { return GetIFP(FHttpHeaders::AcceptCharset()); }
FStringView FHttpHeader::HTTP_AcceptEncoding() const NOEXCEPT { return GetIFP(FHttpHeaders::AcceptEncoding()); }
FStringView FHttpHeader::HTTP_AcceptLanguage() const NOEXCEPT { return GetIFP(FHttpHeaders::AcceptLanguage()); }
FStringView FHttpHeader::HTTP_AccessControlAllowOrigin() const NOEXCEPT { return GetIFP(FHttpHeaders::AccessControlAllowOrigin()); }
FStringView FHttpHeader::HTTP_CacheControl() const NOEXCEPT { return GetIFP(FHttpHeaders::CacheControl()); }
FStringView FHttpHeader::HTTP_Connection() const NOEXCEPT { return GetIFP(FHttpHeaders::Connection()); }
FStringView FHttpHeader::HTTP_Cookie() const NOEXCEPT { return GetIFP(FHttpHeaders::Cookie()); }
FStringView FHttpHeader::HTTP_ContentLanguage() const NOEXCEPT { return GetIFP(FHttpHeaders::ContentLanguage()); }
FStringView FHttpHeader::HTTP_ContentLength() const NOEXCEPT { return GetIFP(FHttpHeaders::ContentLength()); }
FStringView FHttpHeader::HTTP_ContentType() const NOEXCEPT { return GetIFP(FHttpHeaders::ContentType()); }
FStringView FHttpHeader::HTTP_Date() const NOEXCEPT { return GetIFP(FHttpHeaders::Date()); }
FStringView FHttpHeader::HTTP_Host() const NOEXCEPT { return GetIFP(FHttpHeaders::Host()); }
FStringView FHttpHeader::HTTP_KeepAlive() const NOEXCEPT { return GetIFP(FHttpHeaders::KeepAlive()); }
FStringView FHttpHeader::HTTP_Location() const NOEXCEPT { return GetIFP(FHttpHeaders::Location()); }
FStringView FHttpHeader::HTTP_Referer() const NOEXCEPT { return GetIFP(FHttpHeaders::Referer()); }
FStringView FHttpHeader::HTTP_RetryAfter() const NOEXCEPT { return GetIFP(FHttpHeaders::RetryAfter()); }
FStringView FHttpHeader::HTTP_Server() const NOEXCEPT { return GetIFP(FHttpHeaders::Server()); }
FStringView FHttpHeader::HTTP_Status() const NOEXCEPT { return GetIFP(FHttpHeaders::Status()); }
FStringView FHttpHeader::HTTP_UserAgent() const NOEXCEPT { return GetIFP(FHttpHeaders::UserAgent()); }
//----------------------------------------------------------------------------
void FHttpHeader::HTTP_SetAccept(FString&& value) { Add(FHttpHeaders::Accept(), std::move(value)); }
void FHttpHeader::HTTP_SetAcceptCharset(FString&& value) { Add(FHttpHeaders::AcceptCharset(), std::move(value)); }
void FHttpHeader::HTTP_SetAcceptEncoding(FString&& value) { Add(FHttpHeaders::AcceptEncoding(), std::move(value)); }
void FHttpHeader::HTTP_SetAcceptLanguage(FString&& value) { Add(FHttpHeaders::AcceptLanguage(), std::move(value)); }
void FHttpHeader::HTTP_SetAccessControlAllowOrigin(FString&& value) { Add(FHttpHeaders::AccessControlAllowOrigin(), std::move(value)); }
void FHttpHeader::HTTP_SetCacheControl(FString&& value) { Add(FHttpHeaders::CacheControl(), std::move(value)); }
void FHttpHeader::HTTP_SetConnection(FString&& value) { Add(FHttpHeaders::Connection(), std::move(value)); }
void FHttpHeader::HTTP_SetCookie(FString&& value) { Add(FHttpHeaders::Cookie(), std::move(value)); }
void FHttpHeader::HTTP_SetContentLanguage(FString&& value) { Add(FHttpHeaders::ContentLanguage(), std::move(value)); }
void FHttpHeader::HTTP_SetContentLength(FString&& value) { Add(FHttpHeaders::ContentLength(), std::move(value)); }
void FHttpHeader::HTTP_SetContentType(FString&& value) { Add(FHttpHeaders::ContentType(), std::move(value)); }
void FHttpHeader::HTTP_SetDate(FString&& value) { Add(FHttpHeaders::Date(), std::move(value)); }
void FHttpHeader::HTTP_SetHost(FString&& value) { Add(FHttpHeaders::Host(), std::move(value)); }
void FHttpHeader::HTTP_SetKeepAlive(FString&& value) { Add(FHttpHeaders::KeepAlive(), std::move(value)); }
void FHttpHeader::HTTP_SetLocation(FString&& value) { Add(FHttpHeaders::Location(), std::move(value)); }
void FHttpHeader::HTTP_SetReferer(FString&& value) { Add(FHttpHeaders::Referer(), std::move(value)); }
void FHttpHeader::HTTP_SetRetryAfter(FString&& value) { Add(FHttpHeaders::RetryAfter(), std::move(value)); }
void FHttpHeader::HTTP_SetServer(FString&& value) { Add(FHttpHeaders::Server(), std::move(value)); }
void FHttpHeader::HTTP_SetStatus(FString&& value) { Add(FHttpHeaders::Status(), std::move(value)); }
void FHttpHeader::HTTP_SetUserAgent(FString&& value) { Add(FHttpHeaders::UserAgent(), std::move(value)); }
//----------------------------------------------------------------------------
void FHttpHeader::HTTP_SetAccept(const FStringView& value) { HTTP_SetAccept(FString{ value }); }
void FHttpHeader::HTTP_SetAcceptCharset(const FStringView& value) { HTTP_SetAcceptCharset(FString{ value }); }
void FHttpHeader::HTTP_SetAcceptEncoding(const FStringView& value) { HTTP_SetAcceptEncoding(FString{ value }); }
void FHttpHeader::HTTP_SetAcceptLanguage(const FStringView& value) { HTTP_SetAcceptLanguage(FString{ value }); }
void FHttpHeader::HTTP_SetAccessControlAllowOrigin(const FStringView& value) { HTTP_SetAccessControlAllowOrigin(FString{ value }); }
void FHttpHeader::HTTP_SetCacheControl(const FStringView& value) { HTTP_SetCacheControl(FString{ value }); }
void FHttpHeader::HTTP_SetConnection(const FStringView& value) { HTTP_SetConnection(FString{ value }); }
void FHttpHeader::HTTP_SetCookie(const FStringView& value) { HTTP_SetCookie(FString{ value }); }
void FHttpHeader::HTTP_SetContentLanguage(const FStringView& value) { HTTP_SetContentLanguage(FString{ value }); }
void FHttpHeader::HTTP_SetContentLength(const FStringView& value) { HTTP_SetContentLength(FString{ value }); }
void FHttpHeader::HTTP_SetContentType(const FStringView& value) { HTTP_SetContentType(FString{ value }); }
void FHttpHeader::HTTP_SetDate(const FStringView& value) { HTTP_SetDate(FString{ value }); }
void FHttpHeader::HTTP_SetHost(const FStringView& value) { HTTP_SetHost(FString{ value }); }
void FHttpHeader::HTTP_SetKeepAlive(const FStringView& value) { HTTP_SetKeepAlive(FString{ value }); }
void FHttpHeader::HTTP_SetLocation(const FStringView& value) { HTTP_SetLocation(FString{ value }); }
void FHttpHeader::HTTP_SetReferer(const FStringView& value) { HTTP_SetReferer(FString{ value }); }
void FHttpHeader::HTTP_SetRetryAfter(const FStringView& value) { HTTP_SetRetryAfter(FString{ value }); }
void FHttpHeader::HTTP_SetServer(const FStringView& value) { HTTP_SetServer(FString{ value }); }
void FHttpHeader::HTTP_SetStatus(const FStringView& value) { HTTP_SetStatus(FString{ value }); }
void FHttpHeader::HTTP_SetUserAgent(const FStringView& value) { HTTP_SetUserAgent(FString{ value }); }
//----------------------------------------------------------------------------
void FHttpHeader::HTTP_SetAccept(const FName& value) { HTTP_SetAccept(value.MakeView()); }
void FHttpHeader::HTTP_SetAcceptCharset(const FName& value) { HTTP_SetAcceptCharset(value.MakeView()); }
void FHttpHeader::HTTP_SetAcceptEncoding(const FName& value) { HTTP_SetAcceptEncoding(value.MakeView()); }
void FHttpHeader::HTTP_SetAcceptLanguage(const FName& value) { HTTP_SetAcceptLanguage(value.MakeView()); }
void FHttpHeader::HTTP_SetAccessControlAllowOrigin(const FName& value) { HTTP_SetAccessControlAllowOrigin(value.MakeView()); }
void FHttpHeader::HTTP_SetCacheControl(const FName& value) { HTTP_SetCacheControl(value.MakeView()); }
void FHttpHeader::HTTP_SetConnection(const FName& value) { HTTP_SetConnection(value.MakeView()); }
void FHttpHeader::HTTP_SetCookie(const FName& value) { HTTP_SetCookie(value.MakeView()); }
void FHttpHeader::HTTP_SetContentLanguage(const FName& value) { HTTP_SetContentLanguage(value.MakeView()); }
void FHttpHeader::HTTP_SetContentLength(const FName& value) { HTTP_SetContentLength(value.MakeView()); }
void FHttpHeader::HTTP_SetContentType(const FName& value) { HTTP_SetContentType(value.MakeView()); }
void FHttpHeader::HTTP_SetDate(const FName& value) { HTTP_SetDate(value.MakeView()); }
void FHttpHeader::HTTP_SetHost(const FName& value) { HTTP_SetHost(value.MakeView()); }
void FHttpHeader::HTTP_SetKeepAlive(const FName& value) { HTTP_SetKeepAlive(value.MakeView()); }
void FHttpHeader::HTTP_SetLocation(const FName& value) { HTTP_SetLocation(value.MakeView()); }
void FHttpHeader::HTTP_SetReferer(const FName& value) { HTTP_SetReferer(value.MakeView()); }
void FHttpHeader::HTTP_SetRetryAfter(const FName& value) { HTTP_SetRetryAfter(value.MakeView()); }
void FHttpHeader::HTTP_SetServer(const FName& value) { HTTP_SetServer(value.MakeView()); }
void FHttpHeader::HTTP_SetStatus(const FName& value) { HTTP_SetStatus(value.MakeView()); }
void FHttpHeader::HTTP_SetUserAgent(const FName& value) { HTTP_SetUserAgent(value.MakeView()); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
