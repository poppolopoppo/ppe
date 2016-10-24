#pragma once

#include "Core.Network/Network.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FUri {
public:
    typedef ASSOCIATIVE_VECTORINSITU(URI, FString, FString, 3) FQueryMap;

    FUri();
    ~FUri();

    // scheme://username@hostname/p/a/t/h?que=ry#anchor

    const FStringView& Scheme() const { return _scheme; }
    const FStringView& Username() const { return _username; }
    const FStringView& Hostname() const { return _hostname; }
    const FStringView& Path() const { return _path; }
    const FStringView& Query() const { return _query; }
    const FStringView& Anchor() const { return _anchor; }

    size_t Port() const { return _port; }

    const FString& Str() const { return _str; }
    FStringView MakeView() const { return MakeStringView(_str); }

    static bool Pack(
        FUri& dst,
        const FStringView& scheme,
        const FStringView& username,
        size_t port,
        const FStringView& hostname,
        const FStringView& path,
        const FQueryMap& query,
        const FStringView& anchor );

    static bool Unpack(FQueryMap& dst, const FUri& src);

    static bool Parse(FUri& dst, FString&& src);
    static bool Parse(FUri& dst, const FStringView& strview);

    static bool Decode(FString& dst, const FStringView& src);
    static bool Encode(FString& dst, const FStringView& src);

private:
    FStringView _scheme;
    FStringView _username;
    FStringView _hostname;
    FStringView _path;
    FStringView _query;
    FStringView _anchor;

    size_t _port;

    FString _str;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Network::FUri& uri ) {
    return oss << uri.Str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

