#pragma once

#include "Network.h"

#include "Container/AssociativeVector.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FUri {
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

    bool IsAbsolute() const { return (not IsRelative()); }
    bool IsRelative() const { return (_hostname.empty()); }

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
    static bool Decode(FTextWriter& dst, const FStringView& src);

    static bool Encode(FString& dst, const FStringView& src);
    static bool Encode(FTextWriter& dst, const FStringView& src);

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
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_NETWORK_API FTextWriter& operator <<(FTextWriter& oss, const Network::FUri& uri);
PPE_NETWORK_API FWTextWriter& operator <<(FWTextWriter& oss, const Network::FUri& uri);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
