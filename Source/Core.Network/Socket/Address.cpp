#include "stdafx.h"

#include "Address.h"

#include "NetworkIncludes.h"

#include "Core/Diagnostic/LastError.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/TextWriter.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAddress::FAddress() : _port(size_t(EServiceName::Any)) {}
//----------------------------------------------------------------------------
FAddress::~FAddress() {}
//----------------------------------------------------------------------------
FAddress::FAddress(const FStringView& host, size_t port) : FAddress(ToString(host), port) {}
//----------------------------------------------------------------------------
FAddress::FAddress(FString&& host, size_t port) : _host(std::move(host)), _port(port) {
    Assert(FString::npos == _host.find_first_of(':'));
}
//----------------------------------------------------------------------------
bool FAddress::IsIPv4() const {
    u8 ipV4[4];
    return ParseIPv4(ipV4, *this);
}
//----------------------------------------------------------------------------
bool FAddress::IPv4(FAddress* paddr, const FStringView& hostname, size_t port/* = DefaultPort */) {
    Assert(paddr);
    Assert(!hostname.empty());

    if (not HostnameToIPv4(paddr->_host, hostname, paddr->_port))
        return false;

    paddr->_port = port;
    return true;
}
//----------------------------------------------------------------------------
FAddress FAddress::Localhost(size_t port/* = DefaultPort */) {
    return FAddress(MakeStringView("127.0.0.1"), port);
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
    for (auto it = first; first != last; ) {
        if (it == last || *it == '.') {
            if (it == first || slot == 4)
                return false;

            i32 n;
            if (not Atoi32(&n, MakeStringView(first, it), 10))
                return false;

            if (n < 0 || n > 255)
                return false;

            ipV4[slot++] = u8(n);

            first = (it == last) ? last : ++it;
        }
        else if (!IsDigit(*it)) {
            return false;
        }
        else {
            ++it;
        }
    }

    return (4 == slot);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool LocalHostName(FString& hostname) {
    char temp[NI_MAXHOST];
    if (::gethostname(temp, NI_MAXHOST) == SOCKET_ERROR) {
        LOG_LAST_ERROR(L"gethostname");
        return false;
    }
    else {
        hostname.assign(MakeCStringView(temp));
        Assert(hostname.size());
        return true;
    }
}
//----------------------------------------------------------------------------
bool HostnameToIPv4(FString& ip, const FStringView& hostname, size_t port) {
    Assert(!hostname.empty());
    Assert(hostname.size() < NI_MAXHOST);

    char nodeName[NI_MAXHOST]; // hostname must be a null terminated string
    hostname.ToNullTerminatedCStr(nodeName);

    char serviceName[16];
    FFixedSizeTextWriter oss(serviceName);
    oss << port;

    struct ::addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IP v4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV; // Port is a string containing a number

    struct ::addrinfo* serviceInfo = nullptr;
    if (int errorCode = ::getaddrinfo(nodeName, serviceName, &hints, &serviceInfo)) {
        LOG(Error, L"[getaddrinfo] error code: {0}, {1}", errorCode, ::gai_strerror(errorCode));
        return false;
    }
    Assert(serviceInfo);

    STATIC_ASSERT(INET_ADDRSTRLEN <= sizeof(nodeName)); // recycle nodeName buffer

    bool succeed = false;
    for (struct ::addrinfo* p = serviceInfo; p; p = p->ai_next) {
        Assert(AF_INET == p->ai_family);
        Assert(p->ai_addr);

        struct ::sockaddr_in* serviceAddr = reinterpret_cast<struct ::sockaddr_in*>(p->ai_addr);

        const char* resolvedIpV4 = ::inet_ntop(
            p->ai_family,
            &serviceAddr->sin_addr,
            nodeName, sizeof(nodeName) );

        if (resolvedIpV4) {
            ip.assign(MakeCStringView(resolvedIpV4));
            Assert(ip.size());
            LOG(Info, L"[HostnameToIPv4] Resolved IPv4 : {0}:{1} -> {2}", hostname, port, ip);
            succeed = true;
            break;
        }
    }

    ::freeaddrinfo(serviceInfo);

    return succeed;
}
//----------------------------------------------------------------------------
bool HostnameToIPv4(FString& ip, const FStringView& hostname, EServiceName service) {
    return HostnameToIPv4(ip, hostname, size_t(service));
}
//----------------------------------------------------------------------------
bool IPv4ToHostname(FString& hostname, const FStringView& ip) {
    Assert(!ip.empty());

    char temp[NI_MAXHOST]; // hostname must be a null terminated string
    ip.ToNullTerminatedCStr(temp);

    ::sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = ::htons(80);

    // if inet_pton couldn't convert ip then return an error
    if (1 != ::inet_pton(AF_INET, temp, &sa.sin_addr) ) {
        LOG_LAST_ERROR(L"inet_pton");
        return false;
    }

    char hostinfo[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    const DWORD ret = ::getnameinfo(
        (struct sockaddr *)&sa, sizeof(sa),
        hostinfo, NI_MAXHOST,
        servInfo, NI_MAXSERV,
        NI_NUMERICSERV );

    // check if gethostbyaddr returned an error
    if (0 != ret) {
        LOG_LAST_ERROR(L"getnameinfo");
        return false;
    }

    hostname.assign(MakeCStringView(hostinfo));

    Assert(hostname.size());
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const Network::FAddress& addr ) {
    return oss << addr.Host() << ':' << addr.Port();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const Network::FAddress& addr ) {
    return oss << addr.Host() << L':' << addr.Port();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
