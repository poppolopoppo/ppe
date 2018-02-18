#include "stdafx.h"

#include "Address.h"

#include "NetworkIncludes.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/StringHashMap.h"
#include "Core/Diagnostic/LastError.h"
#include "Core/Diagnostic/Logger.h"
#include "Core/IO/String.h"
#include "Core/IO/TextWriter.h"
#include "Core/Meta/Optional.h"
#include "Core/Meta/Singleton.h"
#include "Core/Thread/ReadWriteLock.h"

#define USE_CORE_NETWORK_DNSCACHE 1

namespace Core {
namespace Network {
EXTERN_LOG_CATEGORY(CORE_NETWORK_API, Network)
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
namespace {
//----------------------------------------------------------------------------
#if USE_CORE_NETWORK_DNSCACHE
class FDNSCache_ : Meta::TSingleton<FDNSCache_> {
public: // TSingleton<>
    typedef Meta::TSingleton<FDNSCache_> parent_type;

    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif

    static void Create() {
        LOG(Network, Debug, L"starting DNS cache ...");
        parent_type::Create();
    }

    static void Destroy() {
        LOG(Network, Debug, L"stopping DNS cache ...");
        parent_type::Destroy();
    }

public: // DNS cache
    Meta::TOptional<FStringView> HostnameToIPv4(const FStringView& hostname) const {
        Assert(not hostname.empty());
        const hash_t h = hash_string(hostname);

        READSCOPELOCK(_barrier);
        const auto it = _hostnameToIpV4.find_like(hostname, h);

        Meta::TOptional<FStringView> result;
        if (_hostnameToIpV4.end() != it)
            result.emplace(it->second.MakeView());

        return result;
    }

    Meta::TOptional<FStringView> IPv4ToHostname(const FStringView& ipV4) const {
        Assert(not ipV4.empty());
        const hash_t h = hash_string(ipV4);

        READSCOPELOCK(_barrier);
        const auto it = _ipV4ToHostname.find_like(ipV4, h);

        Meta::TOptional<FStringView> result;
        if (_ipV4ToHostname.end() != it)
            result.emplace(it->second.MakeView());

        return result;
    }

    void PutHostnameToIPv4(const FStringView& hostname, const FStringView& ipV4) {
        Assert(not hostname.empty());
        Assert(not ipV4.empty());

        FString key(hostname);
        FString value(ipV4);

        LOG(Network, Debug, L"put '{0}' => [{1}] in DNS cache", hostname, ipV4);

        WRITESCOPELOCK(_barrier);

        _hostnameToIpV4.insert_or_assign(MakePair(std::move(key), std::move(value)));
    }

    void PutIPv4ToHostname(const FStringView& ipV4, const FStringView& hostname) {
        Assert(not ipV4.empty());
        Assert(not hostname.empty());

        FString key(ipV4);
        FString value(hostname);

        LOG(Network, Debug, L"put [{0}] => '{1}' in DNS cache", ipV4, hostname);

        WRITESCOPELOCK(_barrier);
        _ipV4ToHostname.insert_or_assign(MakePair(std::move(key), std::move(value)));
    }

    void Flush() {
        LOG(Network, Debug, L"flushing DNS cache ...");

        WRITESCOPELOCK(_barrier);
        _hostnameToIpV4.clear();
        _ipV4ToHostname.clear();
    }

private:
    FReadWriteLock _barrier;
    STRING_HASHMAP(DNS, FString, ECase::Sensitive) _hostnameToIpV4;
    STRING_HASHMAP(DNS, FString, ECase::Sensitive) _ipV4ToHostname;
};
#endif //!USE_CORE_NETWORK_DNSCACHE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
void FAddress::Start() {
#if USE_CORE_NETWORK_DNSCACHE
    FDNSCache_::Create();
#endif
}
//----------------------------------------------------------------------------
void FAddress::Shutdown() {
#if USE_CORE_NETWORK_DNSCACHE
    FDNSCache_::Destroy();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FlushDNSCache() {
#if USE_CORE_NETWORK_DNSCACHE
    FDNSCache_::Instance().Flush();
#endif
}
//----------------------------------------------------------------------------
bool LocalHostName(FString& hostname) {
    char temp[NI_MAXHOST];
    if (::gethostname(temp, NI_MAXHOST) == SOCKET_ERROR) {
        LOG_WSALASTERROR(L"gethostname()");
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

#if USE_CORE_NETWORK_DNSCACHE
    {
        const FStringView cacheKey = FStringView(buffer, offEnd);
        if (const Meta::TOptional<FStringView> cacheValue = FDNSCache_::Instance().HostnameToIPv4(cacheKey)) {
            ip.assign(*cacheValue);
            return (not ip.empty()); // negative queries are also cached
        }
    }
#endif

    const char* nodeName = buffer;
    const char* serviceName = &buffer[offSep + 1];
    buffer[offSep] = '\0';

    struct ::addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IP v4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV; // Port is a string containing a number

    struct ::addrinfo* serviceInfo = nullptr;
    if (int errorCode = ::getaddrinfo(nodeName, serviceName, &hints, &serviceInfo)) {
        LOG(Network, Error, L"getaddrinfo failed: <{0}> \"{1}\"", errorCode, ::gai_strerror(errorCode));
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
            LOG(Network, Info, L"resolved IPv4 : {0}:{1} -> {2}", hostname, port, ip);
            succeed = true;
            break;
        }
    }

    ::freeaddrinfo(serviceInfo);

#if USE_CORE_NETWORK_DNSCACHE
    {
        hostnameWPort.Reset();
        hostnameWPort << hostname << ':' << port;
        FDNSCache_::Instance().PutHostnameToIPv4(hostnameWPort.Written(), succeed ? ip.MakeView() : FStringView());
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

#if USE_CORE_NETWORK_DNSCACHE
    if (const Meta::TOptional<FStringView> cacheValue = FDNSCache_::Instance().IPv4ToHostname(ip)) {
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
        LOG_WSALASTERROR(L"inet_pton()");
#if USE_CORE_NETWORK_DNSCACHE
        FDNSCache_::Instance().PutHostnameToIPv4(ip, FStringView());
#endif
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
        LOG_WSALASTERROR(L"getnameinfo()");
#if USE_CORE_NETWORK_DNSCACHE
        FDNSCache_::Instance().PutHostnameToIPv4(ip, FStringView());
#endif
        return false;
    }

    hostname.assign(MakeCStringView(hostinfo));

#if USE_CORE_NETWORK_DNSCACHE
    FDNSCache_::Instance().PutHostnameToIPv4(ip, hostname.MakeView());
#endif

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
