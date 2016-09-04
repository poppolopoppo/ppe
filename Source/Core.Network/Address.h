#pragma once

#include "Core.Network/Network.h"

#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Address {
public:
    STATIC_CONST_INTEGRAL(size_t, DefaultPort, 80);

    Address();
    Address(const StringView& host, size_t port);
    Address(String&& host, size_t port);
    explicit Address(size_t port) : Address(String(), port) {}
    ~Address();

    Address(const Address& ) = default;
    Address& operator =(const Address& ) = default;

    Address(Address&& ) = default;
    Address& operator =(Address&& ) = default;

    bool empty() const { return (_host.empty()); }

    const String& Host() const { return _host; }
    void SetHost(String&& value) { _host = std::move(value); }
    void SetHost(const StringView& value) { _host = ToString(value); }

    size_t Port() const { return _port; }
    void SetPort(size_t value) { _port = value; }

    bool IsIPv4() const;

    static bool IP(Address* paddr, const StringView& hostname, size_t port = DefaultPort);
    static bool Localhost(Address* paddr, size_t port = DefaultPort);
    static bool Parse(Address* paddr, const StringView& input);

    static bool ParseIPv4(u8 (&ipV4)[4], const Address& addr);

    inline friend bool operator ==(const Address& lhs, const Address& rhs) {
        return (lhs.Port() == rhs.Port() && lhs.Host() == rhs.Host());
    }
    inline friend bool operator !=(const Address& lhs, const Address& rhs) {
        return not operator ==(lhs, rhs);
    }

    inline friend bool operator < (const Address& lhs, const Address& rhs) {
        const int c = lhs.Host().compare(rhs.Host());
        return (0 == c ? lhs.Port() < rhs.Port() : c < 0);
    }
    inline friend bool operator >=(const Address& lhs, const Address& rhs) {
        return (not operator <(lhs, rhs));
    }

    template <typename _Char, typename _Traits>
    inline friend std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Address& a) {
        return oss << a.Host() << ':' << a.Port();
    }

private:
    String _host;
    size_t _port;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool LocalHostName(String& hostname);
//----------------------------------------------------------------------------
bool HostnameToIP(String& ip, const StringView& hostname, size_t n = 0);
//----------------------------------------------------------------------------
bool IPToHostname(String& hostname, const StringView& ip);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
