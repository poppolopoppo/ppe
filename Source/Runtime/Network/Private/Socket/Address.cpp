﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Socket/Address.h"

#include "NetworkIncludes.h"

#include "Container/HashMap.h"
#include "Container/StringHashMap.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Meta/Optional.h"
#include "Meta/Singleton.h"
#include "Thread/ReadWriteLock.h"

#define USE_PPE_NETWORK_DNSCACHE 1

namespace PPE {
namespace Network {
EXTERN_LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAddress::FAddress() : _port(size_t(EServiceName::Any)) {}
//----------------------------------------------------------------------------
FAddress::~FAddress() = default;
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
            if (not Atoi(&n, MakeStringView(first, it), 10))
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
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_NETWORK_DNSCACHE
class FDNSCache_ : Meta::TStaticSingleton<FDNSCache_> {
    friend Meta::TStaticSingleton<FDNSCache_>;
    using singleton_type = Meta::TStaticSingleton<FDNSCache_>;
public: // TSingleton<>

    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static void Create() {
        PPE_LOG(Network, Debug, "starting DNS cache ...");
        singleton_type::Create();
    }

    static void Destroy() {
        PPE_LOG(Network, Debug, "stopping DNS cache ...");
        singleton_type::Destroy();
    }

public: // DNS cache
    Meta::TOptional<FStringView> HostnameToIPv4(const FStringView& hostname) const {
        Assert(not hostname.empty());
        const hash_t h = hash_string(hostname);

        READSCOPELOCK(_barrier);
        const auto it = _hostnameToIpV4.find_like(hostname, h);

        Meta::TOptional<FStringView> result;
        if (_hostnameToIpV4.end() != it) {
            PPE_LEAKDETECTOR_WHITELIST_SCOPE();
            result.emplace(it->second.MakeView());
        }

        return result;
    }

    Meta::TOptional<FStringView> IPv4ToHostname(const FStringView& ipV4) const {
        Assert(not ipV4.empty());
        const hash_t h = hash_string(ipV4);

        READSCOPELOCK(_barrier);
        const auto it = _ipV4ToHostname.find_like(ipV4, h);

        Meta::TOptional<FStringView> result;
        if (_ipV4ToHostname.end() != it) {
            PPE_LEAKDETECTOR_WHITELIST_SCOPE();
            result.emplace(it->second.MakeView());
        }

        return result;
    }

    void PutHostnameToIPv4(const FStringView& hostname, const FStringView& ipV4) {
        Assert(not hostname.empty());
        Assert(not ipV4.empty());

        PPE_LEAKDETECTOR_WHITELIST_SCOPE();

        FString key(hostname);
        FString value(ipV4);

        PPE_LOG(Network, Debug, "put '{0}' => [{1}] in DNS cache", hostname, ipV4);

        WRITESCOPELOCK(_barrier);

        _hostnameToIpV4.insert_or_assign(MakePair(std::move(key), std::move(value)));
    }

    void PutIPv4ToHostname(const FStringView& ipV4, const FStringView& hostname) {
        Assert(not ipV4.empty());
        Assert(not hostname.empty());

        PPE_LEAKDETECTOR_WHITELIST_SCOPE();

        FString key(ipV4);
        FString value(hostname);

        PPE_LOG(Network, Debug, "put [{0}] => '{1}' in DNS cache", ipV4, hostname);

        WRITESCOPELOCK(_barrier);
        _ipV4ToHostname.insert_or_assign(MakePair(std::move(key), std::move(value)));
    }

    void Flush() {
        PPE_LOG(Network, Debug, "flushing DNS cache ...");

        PPE_LEAKDETECTOR_WHITELIST_SCOPE();

        WRITESCOPELOCK(_barrier);
        _hostnameToIpV4.clear_ReleaseMemory();
        _ipV4ToHostname.clear_ReleaseMemory();
    }

private:
    FReadWriteLock _barrier;
    STRING_HASHMAP(DNS, FString, ECase::Sensitive) _hostnameToIpV4;
    STRING_HASHMAP(DNS, FString, ECase::Sensitive) _ipV4ToHostname;
};
#endif //!USE_PPE_NETWORK_DNSCACHE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
void FAddress::Start() {
#if USE_PPE_NETWORK_DNSCACHE
    FDNSCache_::Create();
#endif
}
//----------------------------------------------------------------------------
void FAddress::Shutdown() {
#if USE_PPE_NETWORK_DNSCACHE
    FDNSCache_::Destroy();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FlushDNSCache() {
#if USE_PPE_NETWORK_DNSCACHE
    FDNSCache_::Get().Flush();
#endif
}
//----------------------------------------------------------------------------
bool LocalHostName(FString& hostname) {
    char temp[NI_MAXHOST];
    if (::gethostname(temp, NI_MAXHOST) == SOCKET_ERROR) {
        PPE_LOG_NETWORKERROR("gethostname()");
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

    char buffer[NI_MAXHOST + 16]; // hostname must be a null terminated string

    FFixedSizeTextWriter hostnameWPort(buffer);
    hostnameWPort << hostname;
    const size_t offSep = hostnameWPort.Tell();
    hostnameWPort << ':' << port;
    const size_t offEnd = hostnameWPort.Tell();
    hostnameWPort << Eos;

#if USE_PPE_NETWORK_DNSCACHE
    {
        const FStringView cacheKey = FStringView(buffer, offEnd);
        if (const Meta::TOptional<FStringView> cacheValue = FDNSCache_::Get().HostnameToIPv4(cacheKey)) {
            ip.assign(*cacheValue);
            return (not ip.empty()); // negative queries are also cached
        }
    }
#endif

    const char* nodeName = buffer;
    const char* serviceName = &buffer[offSep + 1];
    buffer[offSep] = '\0';

    struct ::addrinfo hints;
    FPlatformMemory::Memzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // IP v4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV; // Port is a string containing a number

    struct ::addrinfo* serviceInfo = nullptr;
    if (int errorCode = ::getaddrinfo(nodeName, serviceName, &hints, &serviceInfo)) {
        PPE_LOG(Network, Error, "getaddrinfo failed: <{0}> \"{1}\"", errorCode, ::gai_strerror(errorCode));
        return false;
    }
    Assert(serviceInfo);

    STATIC_ASSERT(INET_ADDRSTRLEN <= sizeof(buffer)); // recycle nodeName buffer

    bool succeed = false;
    for (struct ::addrinfo* p = serviceInfo; p; p = p->ai_next) {
        Assert(AF_INET == p->ai_family);
        Assert(p->ai_addr);

        struct ::sockaddr_in* serviceAddr = reinterpret_cast<struct ::sockaddr_in*>(p->ai_addr);

        const char* resolvedIpV4 = ::inet_ntop(
            p->ai_family,
            &serviceAddr->sin_addr,
            buffer, sizeof(buffer) );

        if (resolvedIpV4) {
            ip.assign(MakeCStringView(resolvedIpV4));
            Assert(ip.size());
            PPE_LOG(Network, Info, "resolved IPv4 : {0}:{1} -> {2}", hostname, port, ip);
            succeed = true;
            break;
        }
    }

    ::freeaddrinfo(serviceInfo);

#if USE_PPE_NETWORK_DNSCACHE
    {
        hostnameWPort.Reset();
        hostnameWPort << hostname << ':' << port;
        FDNSCache_::Get().PutHostnameToIPv4(hostnameWPort.Written(), succeed ? ip.MakeView() : FStringView());
    }
#endif

    return succeed;
}
//----------------------------------------------------------------------------
bool HostnameToIPv4(FString& ip, const FStringView& hostname, EServiceName service) {
    return HostnameToIPv4(ip, hostname, size_t(service));
}
//----------------------------------------------------------------------------
bool IPv4ToHostname(FString& hostname, const FStringView& ip) {
    Assert(!ip.empty());

#if USE_PPE_NETWORK_DNSCACHE
    if (const Meta::TOptional<FStringView> cacheValue = FDNSCache_::Get().IPv4ToHostname(ip)) {
        hostname.assign(*cacheValue);
        return (not hostname.empty()); // negative queries are also cached
    }
#endif

    char temp[32]; // hostname must be a null terminated string
    ip.ToNullTerminatedCStr(temp);

    ::sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = ::htons(80);

    // if inet_pton couldn't convert ip then return an error
    if (1 != ::inet_pton(AF_INET, temp, &sa.sin_addr) ) {
        PPE_LOG_NETWORKERROR("inet_pton()");
#if USE_PPE_NETWORK_DNSCACHE
        FDNSCache_::Get().PutHostnameToIPv4(ip, FStringView());
#endif
        return false;
    }

    char hostinfo[NI_MAXHOST];
    char servInfo[NI_MAXSERV];

    const int ret = ::getnameinfo(
        (struct sockaddr *)&sa, sizeof(sa),
        hostinfo, NI_MAXHOST,
        servInfo, NI_MAXSERV,
        NI_NUMERICSERV );

    // check if gethostbyaddr returned an error
    if (0 != ret) {
        PPE_LOG_NETWORKERROR("getnameinfo()");
#if USE_PPE_NETWORK_DNSCACHE
        FDNSCache_::Get().PutHostnameToIPv4(ip, FStringView());
#endif
        return false;
    }

    hostname.assign(MakeCStringView(hostinfo));

#if USE_PPE_NETWORK_DNSCACHE
    FDNSCache_::Get().PutHostnameToIPv4(ip, hostname.MakeView());
#endif

    Assert(hostname.size());
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FAddress& addr ) {
    return oss << addr.Host() << ':' << addr.Port();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FAddress& addr ) {
    return oss << addr.Host() << L':' << addr.Port();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
