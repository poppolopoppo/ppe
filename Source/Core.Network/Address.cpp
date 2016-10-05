#include "stdafx.h"

#include "Address.h"

#include "NetworkIncludes.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAddress::FAddress() : _port(DefaultPort) {}
//----------------------------------------------------------------------------
FAddress::FAddress(const FStringView& host, size_t port) : FAddress(ToString(host), port) {}
//----------------------------------------------------------------------------
FAddress::FAddress(FString&& host, size_t port) : _host(std::move(host)), _port(port) {
    Assert(FString::npos == _host.find_first_of(':'));
}
//----------------------------------------------------------------------------
FAddress::~FAddress() {}
//----------------------------------------------------------------------------
bool FAddress::IsIPv4() const {
    u8 ipV4[4];
    return ParseIPv4(ipV4, *this);
}
//----------------------------------------------------------------------------
bool FAddress::IP(FAddress* paddr, const FStringView& hostname, size_t port/* = DefaultPort */) {
    Assert(paddr);
    Assert(!hostname.empty());

    if (not HostnameToIP(paddr->_host, hostname))
        return false;

    paddr->_port = port;
    return true;
}
//----------------------------------------------------------------------------
bool FAddress::Localhost(FAddress* paddr, size_t port/* = DefaultPort */) {
    Assert(paddr);

    if (not LocalHostName(paddr->_host))
        return false;

    paddr->_port = port;
    return true;
}
//----------------------------------------------------------------------------
bool FAddress::Parse(FAddress* paddr, const FStringView& input) {
    Assert(paddr);
    Assert(!input.empty());

    const auto it = StrRChr(input, ':');
    if (it == input.rend())
        return false;

    const size_t length = std::distance(input.rbegin(), it);
    const FStringView inputPort(std::addressof(*it), length);
    Assert(!inputPort.empty());

    if (not Atoi(&paddr->_port, inputPort, 10))
        return false;

    paddr->_host.assign(std::addressof(*input.begin()), std::addressof(*it));

    return true;
}
//----------------------------------------------------------------------------
bool FAddress::ParseIPv4(u8 (&ipV4)[4], const FAddress& addr) {
    Assert(!addr.empty());

    size_t slot = 0;

    auto first = addr._host.begin();
    const auto last = addr._host.end();
    for (auto it = first; first < last; ++it) {
        if (*it == '.' || it == last) {
            if (it == first || slot == 4)
                return false;

            i32 n;
            if (not Atoi32(&n, MakeStringView(first, it), 10))
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
bool LocalHostName(FString& hostname) {
    char temp[NI_MAXHOST];
    if (::gethostname(temp, NI_MAXHOST) == SOCKET_ERROR)
        return false;

    hostname.assign(temp);

    Assert(hostname.size());
    return true;
}
//----------------------------------------------------------------------------
bool HostnameToIP(FString& ip, const FStringView& hostname, size_t n/* = 0 */) {
    Assert(!hostname.empty());
    Assert(hostname.size() < NI_MAXHOST);

    char nodeName[NI_MAXHOST]; // hostname must be a null terminated string
    hostname.ToNullTerminatedCStr(nodeName);

    const char* serviceName = "http"; // use http as default port

    ::ADDRINFOA* address;
    if (0 != ::getaddrinfo(nodeName, serviceName, nullptr, &address))
        return false;

    // find the nth address
    ::addrinfo* nth_addr = address;
    for (size_t i = 1; i <= n; ++i) {
        nth_addr = nth_addr->ai_next;

        // if there is no nth address then return error
        if (nullptr == nth_addr) {
            ::freeaddrinfo(address);
            return false;
        }
    }

    char temp_ip[17];
    const char* resolved_ip = ::inet_ntop(nth_addr->ai_family, nth_addr->ai_addr, temp_ip, lengthof(temp_ip));

    ::freeaddrinfo(address);

    if (nullptr == resolved_ip)
        return false;

    ip.assign(resolved_ip);

    Assert(ip.size());
    return true;
}
//----------------------------------------------------------------------------
bool IPToHostname(FString& hostname, const FStringView& ip) {
    Assert(!ip.empty());

    char temp[NI_MAXHOST]; // hostname must be a null terminated string
    ip.ToNullTerminatedCStr(temp);

    ::sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = ::htons(80);

    // if inet_pton couldn't convert ip then return an error
    if (1 != ::inet_pton(AF_INET, temp, &sa.sin_addr) )
        return false;

    char hostinfo[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    const DWORD ret = ::getnameinfo(
        (struct sockaddr *)&sa, sizeof(sa),
            hostinfo, NI_MAXHOST,
            servInfo, NI_MAXSERV,
            NI_NUMERICSERV );

    // check if gethostbyaddr returned an error
    if (0 != ret)
        return false;

    hostname.assign(hostinfo);

    Assert(hostname.size());
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
