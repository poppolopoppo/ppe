#include "stdafx.h"

#include "Header.h"

#include "../Socket/SocketBuffered.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool HeaderReadUntil_(std::ostream* poss, FSocketBuffered& socket, const char delim = '\n') {
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
}
//----------------------------------------------------------------------------
bool FHttpHeader::Read(FHttpHeader* pheader, FSocketBuffered& socket) {
    Assert(pheader);
    Assert(socket.IsConnected());

    STACKLOCAL_OCSTRSTREAM(oss, 1024);

    char ch;
    while (socket.Peek(ch)) {
        HeaderReadUntil_(&oss, socket);

        const FStringView line = Strip(oss.MakeView());
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
FStringView FHttpHeader::ProtocolVersion() {
    return "HTTP/1.1";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
