#include "stdafx.h"

#include "Socket.h"

// Socket implementation adapted/updated from C++ DLib
// http://dlib.net/files/dlib-19.1.zip
// http://dlib.net/dlib/sockets/sockets_kernel_1.h.html

#ifdef OS_WINDOWS

#   ifndef _MSC_VER
#       error "This source file is intended for Visual Studio"
#   endif

#   include "Core/Thread/AtomicSpinLock.h"

#   include <WinSock2.h>
#   include <Ws2tcpip.h>

#   ifndef _WINSOCKAPI_
#       error "_WINSOCKAPI_ must be defined to prevent inclusion of winsock.h"
#   endif

#   include <windows.h>

#   pragma comment (lib, "ws2_32.lib")

#else

#   error "TODO: fix includes, but should be pretty close to the windows version (since M$ stole unix network layer)"

#endif

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, MaxRecvLength_, 1024*1024*100);
STATIC_CONST_INTEGRAL(size_t, MaxSendLength_, 1024*1024*100);
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(::SOCKET) <= sizeof(void*));
STATIC_ASSERT(std::is_pod<::SOCKET>::value);
//----------------------------------------------------------------------------
static void* PackSocket_(::SOCKET socket) {
    return reinterpret_cast<void*>(socket);
}
//----------------------------------------------------------------------------
static ::SOCKET UnpackSocket_(void* handle) {
    Assert(handle);
    STATIC_ASSERT(sizeof(::SOCKET) <= sizeof(handle));
    return reinterpret_cast<::SOCKET>(handle);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Socket::Socket() : Socket(Address(), Address()) {}
//----------------------------------------------------------------------------
Socket::Socket(Address&& remote, Address&& local)
:   _handle(nullptr)
,   _userData(nullptr)
,   _remote(std::move(remote))
,   _local(std::move(local)) {
    Assert(_local.empty() || _local.IsIPv4());
    Assert(_remote.IsIPv4());
}
//----------------------------------------------------------------------------
Socket::~Socket() {
    Assert(!IsConnected());
}
//----------------------------------------------------------------------------
Socket& Socket::operator =(Socket&& rvalue) {
    if (IsConnected())
        Disconnect();

    _handle = rvalue._handle;
    _local = std::move(rvalue._local);
    _remote = std::move(rvalue._remote);

    rvalue._handle = nullptr;

    return *this;
}
//----------------------------------------------------------------------------
bool Socket::Connect() {
    Assert(!IsConnected());
    Assert(!_remote.empty());

    ::sockaddr_in local_sa;     // local socket structure
    ::sockaddr_in foreign_sa;   // foreign socket structure
    ::ZeroMemory(&local_sa, sizeof(sockaddr_in));   // initialize local_sa
    ::ZeroMemory(&foreign_sa, sizeof(sockaddr_in)); // initialize foreign_sa

    ::SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);  // get a new socket

    // if socket() returned an error then return OTHER_ERROR
    if (INVALID_SOCKET == sock)
        return false;

    // set the foreign socket structure
    foreign_sa.sin_family = AF_INET;
    foreign_sa.sin_port = ::htons(checked_cast<::u_short>(_remote.Port()));

    // if inet_pton couldn't convert the ip then return an error
    if (1 != ::inet_pton(AF_INET, _remote.Host().c_str(), &foreign_sa.sin_addr.S_un.S_addr) ) {
        ::closesocket(sock);
        return false;
    }

    // set up the local socket structure
    local_sa.sin_family = AF_INET;

    // set the local ip
    if (_local.empty()) {
        // if the listener should listen on any IP
        local_sa.sin_addr.S_un.S_addr = ::htons(INADDR_ANY);
    }
    else {
        Assert(_local.IsIPv4());

        // if there is a specific ip to listen on
        // if inet_pton couldn't convert the ip then return an error
        if (1 != ::inet_pton(AF_INET, _local.Host().c_str(), &local_sa.sin_addr.S_un.S_addr) ) {
            ::closesocket(sock);
            return false;
        }
    }

    // set the local port
    local_sa.sin_port = ::htons(checked_cast<::u_short>(_local.Port()));

    // bind the new socket to the requested local port and local ip
    if (SOCKET_ERROR == ::bind(sock, reinterpret_cast<sockaddr*>(&local_sa), sizeof(sockaddr_in)) ) {
        ::closesocket(sock);
        return false;
    }

    // connect the socket
    if (SOCKET_ERROR == ::connect(sock, reinterpret_cast<sockaddr*>(&foreign_sa), sizeof(sockaddr_in)) ) {
        ::closesocket(sock);
        return false;
    }

    // determine the local port and IP and store them in used_local_ip
    // and used_local_port
    ::u_short used_local_port;
    ::sockaddr_in local_info;
    String used_local_ip;
    if (0 == _local.Port()) {
        int length = sizeof(::sockaddr_in);
        if (SOCKET_ERROR == ::getsockname(sock, reinterpret_cast<::sockaddr*>(&local_info), &length)) {
            ::closesocket(sock);
            return false;
        }

        used_local_port = ::ntohs(local_info.sin_port);
    }
    else {
        used_local_port = checked_cast<::u_short>(_local.Port());
    }

    // determine real local ip
    if (_local.empty()) {
        // if local_port is not 0 then we must fill the local_info structure
        if (_local.Port() != 0) {
            int length = sizeof(sockaddr_in);
            if (SOCKET_ERROR == ::getsockname(sock, reinterpret_cast<::sockaddr*>(&local_info), &length) ) {
                ::closesocket(sock);
                return false;
            }
        }

        char temp[17];
        const char* real_local_ip = ::inet_ntop(AF_INET, &local_info.sin_addr, temp, lengthof(temp));

        // check if inet_ntop returned an error
        if (nullptr == real_local_ip) {
            ::closesocket(sock);
            return false;
        }

        used_local_ip.assign(real_local_ip);
    }
    else {
        Assert(_local.IsIPv4());

        used_local_ip = _local.Host();
    }

    // set the SO_OOBINLINE option
    int flag_value = 1;
    if (SOCKET_ERROR == ::setsockopt(sock, SOL_SOCKET, SO_OOBINLINE, reinterpret_cast<const char*>(&flag_value), sizeof(int)) ) {
        ::closesocket(sock);
        return false;
    }

    _handle = PackSocket_(sock);
    _remote = Address(std::move(used_local_ip), used_local_port);

    Assert(IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool Socket::Disconnect(bool gracefully/* = false */) {
    Assert(IsConnected());

    ::SOCKET sock = UnpackSocket_(_handle);
    _handle = nullptr;

    if (gracefully) {
        // shutdown every outgoing packets
        if (ShutdownOutgoing()) {
            // wait for the other end to close their side
            u8 junk[100];
            while (Read(MakeView(junk)) > 0);
        }
    }

    const int status = ::closesocket(sock);
    if (status == -1)
        return false;

    Assert(!IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool Socket::DisableNagle() {
    Assert(IsConnected());

    int flag = 1;
    const int status = ::setsockopt(UnpackSocket_(_handle), IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));

    return (SOCKET_ERROR != status);
}
//----------------------------------------------------------------------------
bool Socket::ShutdownOutgoing() {
    Assert(IsConnected());

    const int status = ::shutdown(UnpackSocket_(_handle), SD_SEND);

    return (status != -1);
}
//----------------------------------------------------------------------------
bool Socket::IsConnected() const {
    return (nullptr != _handle && INVALID_SOCKET != UnpackSocket_(_handle));
}
//----------------------------------------------------------------------------
bool Socket::IsReadable(const Milliseconds& timeout) const {
    Assert(IsConnected());
    Assert(timeout.Value() >= 0);

    ::fd_set read_set;
    // initialize read_set
    FD_ZERO(&read_set);

    // add the listening socket to read_set
    FD_SET(UnpackSocket_(_handle), &read_set);

    // setup a timeval structure
    ::timeval time_to_wait;
    time_to_wait.tv_sec = static_cast<long>(timeout.Value()) / 1000;
    time_to_wait.tv_usec = (static_cast<long>(timeout.Value()) % 1000) *1000;

    // wait on select
    const int status = ::select(0, &read_set, 0, 0, &time_to_wait);

    // if select timed out or there was an error
    if (status <= 0)
        return false;

    // data is ready to be read
    return true;
}
//----------------------------------------------------------------------------
size_t Socket::Read(MemoryView<u8>& rawData) {
    Assert(!rawData.empty());
    Assert(IsConnected());

    // Make sure to cap the max value num can take on so that if it is
    // really large (it might be big on 64bit platforms) so that the OS
    // can't possibly get upset about it being large.
    const int length = (int)Min(MaxRecvLength_, rawData.size());

    const int status = ::recv(UnpackSocket_(_handle), (char*)rawData.data(), length, 0);

    return (SOCKET_ERROR == status ? 0 : status);
}
//----------------------------------------------------------------------------
size_t Socket::Read(MemoryView<u8>& rawData, const Milliseconds& timeout) {
    Assert(!rawData.empty());

    if (not IsReadable(timeout))
        return 0;
    else
        return Read(rawData);
}
//----------------------------------------------------------------------------
size_t Socket::Write(const MemoryView<const u8>& rawData) {
    Assert(!rawData.empty());
    Assert(IsConnected());

    ::SOCKET sock = UnpackSocket_(_handle);

    for (size_t offset = 0; offset < rawData.size(); ) {
        // Make sure to cap the max value num can take on so that if it is
        // really large (it might be big on 64bit platforms) so that the OS
        // can't possibly get upset about it being large.
        const int length = (int)Min(MaxSendLength_, rawData.size() - offset);

        const int status = ::send(sock, (const char*)rawData.data() + offset, length, 0);
        if (SOCKET_ERROR == status)
            return false;

        offset += checked_cast<size_t>(status);
    }

    return rawData.size();
}
//----------------------------------------------------------------------------
bool Socket::MakeConnection(Socket& socket, const Address& remoteHostnameOrIP) {
    Assert(!socket.IsConnected());
    Assert(!remoteHostnameOrIP.empty());

    socket._local = Address();

    if (remoteHostnameOrIP.IsIPv4()) {
        socket._remote = remoteHostnameOrIP;
    }
    else {
        String ip;
        if (not HostnameToIP(ip, MakeView(remoteHostnameOrIP.Host())) )
            return false;

        socket._remote = std::move( Address(std::move(ip), remoteHostnameOrIP.Port()) );
    }
    Assert(socket._remote.IsIPv4());

    return socket.Connect();
}
//----------------------------------------------------------------------------
void Socket::Start() {
#ifdef OS_WINDOWS
    ::WSADATA wsaData;
    if (0 != ::WSAStartup(MAKEWORD(2,2), &wsaData))
        AssertNotReached();
#endif
}
//----------------------------------------------------------------------------
void Socket::Shutdown() {
#ifdef OS_WINDOWS
    ::WSACleanup();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Listener::Listener() : _handle(nullptr) {}
//----------------------------------------------------------------------------
Listener::Listener(Address&& listening)
:   _handle(nullptr)
,   _listening(std::move(listening)) {
    Assert(_listening.IsIPv4());
}
//----------------------------------------------------------------------------
Listener::~Listener() {
    Assert(!IsConnected());
}
//----------------------------------------------------------------------------
Listener& Listener::operator =(Listener&& rvalue) {
    if (IsConnected())
        Disconnect();

    _handle = rvalue._handle;
    _listening = std::move(rvalue._listening);

    rvalue._handle = nullptr;

    return *this;
}
//----------------------------------------------------------------------------
bool Listener::Connect() {
    Assert(!IsConnected());

    ::sockaddr_in sa;  // local socket structure
    ZeroMemory(&sa, sizeof(::sockaddr_in)); // initialize sa

    ::SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);  // get a new socket

    // if socket() returned an error then return OTHER_ERROR
    if (INVALID_SOCKET == sock)
        return false;

    // set the local socket structure
    sa.sin_family = AF_INET;
    sa.sin_port = ::htons(checked_cast<::u_short>(_listening.Port()));
    if (_listening.Host().empty()) {
        // if the listener should listen on any IP
        sa.sin_addr.S_un.S_addr = ::htons(INADDR_ANY);
    }
    else {
        Assert(_listening.IsIPv4());

        // if there is a specific ip to listen on
        // if inet_pton couldn't convert the ip then return an error
        if (1 != ::inet_pton(AF_INET, _listening.Host().c_str(), &sa.sin_addr.S_un.S_addr) ) {
            ::closesocket(sock);
            return false;
        }
    }

    // set the SO_REUSEADDR option
    int flag_value = 1;
    ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&flag_value), sizeof(int));

    // bind the new socket to the requested port and ip
    if (SOCKET_ERROR == ::bind(sock, reinterpret_cast<sockaddr*>(&sa), sizeof(::sockaddr_in)) ) {
        ::closesocket(sock);
        return false;
    }

    // tell the new socket to listen
    if (SOCKET_ERROR == ::listen(sock, SOMAXCONN)) {
        ::closesocket(sock);
        return false;
    }

    // determine the port used if necessary
    if (_listening.Port() == 0) {
        ::sockaddr_in local_info;
        int length = sizeof(::sockaddr_in);
        if (SOCKET_ERROR == ::getsockname (sock, reinterpret_cast<::sockaddr*>(&local_info), &length) ) {
            ::closesocket(sock);
            return false;
        }

        const ::u_short port = ::ntohs(local_info.sin_port);
        _listening.SetPort(port);
    }

    _handle = PackSocket_(sock);

    Assert(IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool Listener::Disconnect() {
    Assert(IsConnected());

    ::SOCKET sock = UnpackSocket_(_handle);
    _handle = nullptr;

    const int status = ::closesocket(sock);
    if (status == -1)
        return false;

    Assert(!IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool Listener::IsConnected() const {
    return (nullptr != _handle && INVALID_SOCKET != UnpackSocket_(_handle));
}
//----------------------------------------------------------------------------
bool Listener::Accept(Socket& socket, const Milliseconds& timeout) {
    Assert(IsConnected());
    Assert(!socket.IsConnected());
    Assert(timeout.Value() >= 0);

    ::SOCKET incoming;
    ::sockaddr_in incomingAddr;
    int length = sizeof(::sockaddr_in);

    // implement timeout with select if timeout is > 0
    if (timeout > 0) {
        ::fd_set read_set;
        // initialize read_set
        FD_ZERO(&read_set);

        // add the listening socket to read_set
        FD_SET(UnpackSocket_(_handle), &read_set);

        // setup a timeval structure
        ::timeval time_to_wait;
        time_to_wait.tv_sec = static_cast<long>(timeout.Value()) / 1000;
        time_to_wait.tv_usec = (static_cast<long>(timeout.Value()) % 1000) *1000;

        // wait on select
        const int status = ::select(0, &read_set, 0, 0, &time_to_wait);

        // if select timed out
        if (0 == status)
            return false;

        // if select returned an error
        if (SOCKET_ERROR == status)
            return false;
    }

    // call accept to get a new connection
    incoming = ::accept(UnpackSocket_(_handle), reinterpret_cast<::sockaddr*>(&incomingAddr), &length);

    // if there was an error return OTHER_ERROR
    if (INVALID_SOCKET == incoming)
        return false;

    // get the port of the foreign host into foreign_port
    int foreign_port = ::ntohs(incomingAddr.sin_port);

    // get the IP of the foreign host into foreign_ip
    String foreign_ip;
    {
        char temp[17];
        const char* real_foreign_ip = ::inet_ntop(AF_INET, &incomingAddr.sin_addr, temp, lengthof(temp));

        // check if inet_ntop() returned an error
        if (nullptr == real_foreign_ip) {
            ::closesocket(incoming);
            return false;
        }

        foreign_ip.assign(real_foreign_ip);
    }

    // get the local ip
    String local_ip;
    if (_listening.empty()) {
        ::sockaddr_in local_info;
        length = sizeof(::sockaddr_in);
        // get the local sockaddr_in structure associated with this new connection
        if (SOCKET_ERROR == ::getsockname (incoming, reinterpret_cast<::sockaddr*>(&local_info), &length) ) {
            // an error occurred
            ::closesocket(incoming);
            return false;
        }

        char temp[17];
        const char* real_local_ip = ::inet_ntop(AF_INET, &local_info.sin_addr, temp, lengthof(temp));

        // check if inet_ntop() returned an error
        if (nullptr == real_local_ip) {
            ::closesocket(incoming);
            return false;
        }

        local_ip.assign(real_local_ip);
    }
    else {
        local_ip = _listening.Host();
    }

    // set the SO_OOBINLINE option
    int flag_value = 1;
    if (SOCKET_ERROR == ::setsockopt(incoming, SOL_SOCKET, SO_OOBINLINE, reinterpret_cast<const char*>(&flag_value), sizeof(int)) ) {
        ::closesocket(incoming);
        return false;
    }

    socket._handle = PackSocket_(incoming);
    socket._local.SetHost(std::move(local_ip));
    socket._local.SetPort(_listening.Port());
    socket._remote.SetHost(std::move(foreign_ip));
    socket._remote.SetPort(foreign_port);

    Assert(socket.IsConnected());
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool LocalHostName(String& hostname) {
    char temp[NI_MAXHOST];
    if (::gethostname(temp, NI_MAXHOST) == SOCKET_ERROR)
        return false;

    hostname.assign(temp);

    Assert(hostname.size());
    return true;
}
//----------------------------------------------------------------------------
bool HostnameToIP(String& ip, const StringSlice& hostname, size_t n/* = 0 */) {
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
    for (int i = 1; i <= n; ++i)
    {
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
bool IPToHostname(String& hostname, const StringSlice& ip) {
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
