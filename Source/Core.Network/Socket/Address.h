#pragma once

#include "Core.Network/Network.h"

#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EServiceName : size_t {
    Any         = 0, // Special value when we don't care about which port we want

    FTP         = 21,
    SSH         = 22,
    Telnet      = 23,
    SMTP        = 25,
    HTTP        = 80,
    POP2        = 109,
    POP3        = 110,
    SFTP        = 115,
    NNTP        = 119,
    HTTPS       = 443,
    SMB         = 445,
    DHCP        = 546,
    RSync       = 873,
    IMAP        = 993,
    OpenVPN     = 1194,
    DDNS        = 2164,
    Jabber      = 5222,
    VNC         = 5900,
    IRC         = 6667,
    Minecraft   = 25565,
    Steam       = 27000,
};
//----------------------------------------------------------------------------
class FAddress {
public:
    FAddress();
    ~FAddress();

    FAddress(const FStringView& host, size_t port);
    FAddress(const FStringView& host, EServiceName service) : FAddress(host, size_t(service)) {}

    FAddress(FString&& host, size_t port);
    FAddress(FString&& host, EServiceName service) : FAddress(std::move(host), size_t(service)) {}

    explicit FAddress(size_t port) : FAddress(FString(), port) {}
    explicit FAddress(EServiceName service) : FAddress(FString(), size_t(service)) {}

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
    // TODO: bool IsIPv6() const;

    static bool IPv4(FAddress* paddr, const FStringView& hostname, size_t port);
    static bool Parse(FAddress* paddr, const FStringView& input);

    static FAddress Localhost(size_t port);

    static bool ParseIPv4(u8 (&ipV4)[4], const FAddress& addr);
    // TODO: static bool ParseIPv6(u8 (&ipV6)[4], const FAddress& addr);

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
bool HostnameToIPv4(FString& ip, const FStringView& hostname, size_t port);
bool HostnameToIPv4(FString& ip, const FStringView& hostname, EServiceName service);
//----------------------------------------------------------------------------
bool IPv4ToHostname(FString& hostname, const FStringView& ip);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const Network::FAddress& addr );
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const Network::FAddress& addr );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
