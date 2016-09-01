#include "stdafx.h"

#include "Address.h"

#include "Socket.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Address::Address() : _port(DefaultPort) {}
//----------------------------------------------------------------------------
Address::Address(const StringSlice& host, size_t port) : Address(ToString(host), port) {}
//----------------------------------------------------------------------------
Address::Address(String&& host, size_t port) : _host(std::move(host)), _port(port) {
    Assert(String::npos == _host.find_first_of(':'));
}
//----------------------------------------------------------------------------
Address::~Address() {}
//----------------------------------------------------------------------------
bool Address::IsIPv4() const {
    u8 ipV4[4];
    return ParseIPv4(ipV4, *this);
}
//----------------------------------------------------------------------------
bool Address::IP(Address* paddr, const StringSlice& hostname, size_t port/* = DefaultPort */) {
    Assert(paddr);
    Assert(!hostname.empty());

    if (not HostnameToIP(paddr->_host, hostname))
        return false;

    paddr->_port = port;
    return true;
}
//----------------------------------------------------------------------------
bool Address::Localhost(Address* paddr, size_t port/* = DefaultPort */) {
    Assert(paddr);

    if (not LocalHostName(paddr->_host))
        return false;

    paddr->_port = port;
    return true;
}
//----------------------------------------------------------------------------
bool Address::Parse(Address* paddr, const StringSlice& input) {
    Assert(paddr);
    Assert(!input.empty());

    const auto it = StrRChr(input, ':');
    if (it == input.rend())
        return false;

    const size_t length = std::distance(input.rbegin(), it);
    const StringSlice inputPort(std::addressof(*it), length);
    Assert(!inputPort.empty());

    if (not Atoi(&paddr->_port, inputPort, 10))
        return false;

    paddr->_host.assign(std::addressof(*input.begin()), std::addressof(*it));

    return true;
}
//----------------------------------------------------------------------------
bool Address::ParseIPv4(u8 (&ipV4)[4], const Address& addr) {
    Assert(!addr.empty());

    size_t slot = 0;

    auto first = addr._host.begin();
    const auto last = addr._host.end();
    for (auto it = first; first < last; ++it) {
        if (*it == '.' || it == last) {
            if (it == first || slot == 4)
                return false;

            i32 n;
            if (not Atoi32(&n, MakeStringSlice(first, it), 10))
                return false;

            if (n < 0 || n > 255)
                return false;

            first = it + 1;
            ipV4[slot++] = u8(n);
        }
        else if (!IsDigit(*it)) {
            return false;
        }
    }

    return (4 == slot);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
