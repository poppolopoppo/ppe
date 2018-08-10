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
FHttpHeader::~FHttpHeader() {}
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
void FHttpHeader::PackCookie(FHttpHeader* pheader, const FCookieMap& cookie) {
    Assert(pheader);

    FStringBuilder oss;

    bool many = false;
    for (const auto& it : cookie) {
        if (many) {
            oss.Put(';');
            oss.Put(' ');
        }

        FUri::Encode(oss, it.first.MakeView());
        oss.Put('=');
        FUri::Encode(oss, it.second.MakeView());

        many = true;
    }

    pheader->Add(FHttpConstNames::Cookie(), oss.ToString());
}
//----------------------------------------------------------------------------
bool FHttpHeader::UnpackCookie(FCookieMap* pcookie, const FHttpHeader& header) {
    Assert(pcookie);

    pcookie->clear();

    FStringView cookieCStr = header.GetIFP(FHttpConstNames::Cookie());
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
void FHttpHeader::PackPost(FHttpHeader* pheader, const FPostMap& post) {
    Assert(pheader);

    pheader->_body.clear();

    FTextWriter oss(&pheader->_body);

    bool many = false;
    for (const auto& it : post) {
        if (many)
            oss.Put('&');

        FUri::Encode(oss, it.first.MakeView());
        oss.Put('=');
        FUri::Encode(oss, it.second.MakeView());

        many = true;
    }

    pheader->Add(FHttpConstNames::ContentType(), FString("application/x-www-form-urlencoded"));
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
