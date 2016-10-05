#pragma once

#include "Core.Network/Network.h"

#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAddress {
public:
    STATIC_CONST_INTEGRAL(size_t, DefaultPort, 80);

    FAddress();
    FAddress(const FStringView& host, size_t port);
    FAddress(FString&& host, size_t port);
    explicit FAddress(size_t port) : FAddress(FString(), port) {}
    ~FAddress();

    FAddress(const FAddress& ) = default;
    FAddress& operator =(const FAddress& ) = default;

    FAddress(FAddress&& ) = default;
    FAddress& operator =(FAddress&& ) = default;

    bool empty() const { return (_host.empty()); }

    const FString& Host() const { return _host; }
    void SetHost(FString&& value) { _host = std::move(value); }
    void SetHost(const FStringView& value) { _host = ToString(value); }

    size_t Port() const { return _port; }
    void SetPort(size_t value) { _port = value; }

    bool IsIPv4() const;

    static bool IP(FAddress* paddr, const FStringView& hostname, size_t port = DefaultPort);
    static bool Localhost(FAddress* paddr, size_t port = DefaultPort);
    static bool Parse(FAddress* paddr, const FStringView& input);

    static bool ParseIPv4(u8 (&ipV4)[4], const FAddress& addr);

    inline friend bool operator ==(const FAddress& lhs, const FAddress& rhs) {
        return (lhs.Port() == rhs.Port() && lhs.Host() == rhs.Host());
    }
    inline friend bool operator !=(const FAddress& lhs, const FAddress& rhs) {
        return not operator ==(lhs, rhs);
    }

    inline friend bool operator < (const FAddress& lhs, const FAddress& rhs) {
        const int c = lhs.Host().compare(rhs.Host());
        return (0 == c ? lhs.Port() < rhs.Port() : c < 0);
    }
    inline friend bool operator >=(const FAddress& lhs, const FAddress& rhs) {
        return (not operator <(lhs, rhs));
    }

    template <typename _Char, typename _Traits>
    inline friend std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const FAddress& a) {
        return oss << a.Host() << ':' << a.Port();
    }

private:
    FString _host;
    size_t _port;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool LocalHostName(FString& hostname);
//----------------------------------------------------------------------------
bool HostnameToIP(FString& ip, const FStringView& hostname, size_t n = 0);
//----------------------------------------------------------------------------
bool IPToHostname(FString& hostname, const FStringView& ip);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
