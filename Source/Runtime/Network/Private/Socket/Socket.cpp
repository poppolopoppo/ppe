﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Socket/Socket.h"

#include "HAL/PlatformMemory.h"
#include "NetworkIncludes.h"

// FSocket implementation adapted/updated from C++ DLib
// http://dlib.net/files/dlib-19.1.zip
// http://dlib.net/dlib/sockets/sockets_kernel_1.h.html

#ifdef PLATFORM_WINDOWS
//  Link with WinSock2 library
#   pragma comment(lib, "ws2_32.lib")
#endif

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static ::timeval MakeTimeval(const FMilliseconds& duration) {
    Assert(duration.Value()  > 0);

    ::timeval time_to_wait;
    time_to_wait.tv_sec = static_cast<long>(duration.Value()) / 1000;
    time_to_wait.tv_usec = (static_cast<long>(duration.Value()) % 1000) * 1000;

    return time_to_wait;
}
//----------------------------------------------------------------------------
static constexpr intptr_t GInvalidSocket_ = PackSocket_(INVALID_SOCKET);
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSocket::FSocket()
:   _handle(GInvalidSocket_)
,   _userData(nullptr)
,   _timeout(double(DefaultTimeoutInMs)) {}
//----------------------------------------------------------------------------
FSocket::FSocket(FAddress&& remote, FAddress&& local)
:   _handle(GInvalidSocket_)
,   _userData(nullptr)
,   _local(std::move(local))
,   _remote(std::move(remote))
,   _timeout(double(DefaultTimeoutInMs)) {
    Assert(_local.empty() || _local.IsIPv4());
    Assert(_remote.IsIPv4());
}
//----------------------------------------------------------------------------
FSocket::~FSocket() {
    Assert(!IsConnected());
}
//----------------------------------------------------------------------------
FSocket& FSocket::operator =(FSocket&& rvalue) NOEXCEPT {
    if (IsConnected())
        Disconnect();

    _handle = rvalue._handle;
    _local = std::move(rvalue._local);
    _remote = std::move(rvalue._remote);
    _timeout = std::move(rvalue._timeout);

    rvalue._handle = GInvalidSocket_;

    return *this;
}
//----------------------------------------------------------------------------
bool FSocket::Connect() {
    Assert(!IsConnected());
    Assert(!_remote.empty());
    Assert(_remote.Port() != size_t(EServiceName::Any));

    // TODO : clearly handle IPv6
    const short ai_family = (_remote.IsIPv4() ? AF_INET : AF_INET6);

    ::sockaddr_in local_sa;     // local socket structure
    ::sockaddr_in foreign_sa;   // foreign socket structure
    FPlatformMemory::Memzero(&local_sa, sizeof(sockaddr_in));   // initialize local_sa
    FPlatformMemory::Memzero(&foreign_sa, sizeof(sockaddr_in)); // initialize foreign_sa

    SOCKET sockfd = ::socket(ai_family, SOCK_STREAM, 0);  // get a new socket

    // if socket() returned an error then return OTHER_ERROR
    if (INVALID_SOCKET == sockfd) {
        PPE_LOG_NETWORKERROR("socket()");
        return false;
    }

    // set the foreign socket structure
    foreign_sa.sin_family = ai_family;
    foreign_sa.sin_port = ::htons(checked_cast<::u_short>(_remote.Port()));

    // if inet_pton couldn't convert the ip then return an error
    if (1 != ::inet_pton(ai_family, _remote.Host().c_str(), &SOCKET_S_ADDR(foreign_sa.sin_addr)) ) {
        PPE_LOG_NETWORKERROR("inet_pton()");
        if (::closesocket(sockfd))
            PPE_LOG_NETWORKERROR("closesocket()");
        return false;
    }

    // set up the local socket structure
    local_sa.sin_family = ai_family;

    // set the local ip
    if (_local.empty()) {
        // if the listener should listen on any IP
        SOCKET_S_ADDR(local_sa.sin_addr) = ::htons(INADDR_ANY);
    }
    else {
        Assert(_local.IsIPv4()); // TODO : clearly handle IPv6

        // if there is a specific ip to listen on
        // if inet_pton couldn't convert the ip then return an error
        if (1 != ::inet_pton(ai_family, _local.Host().c_str(), &SOCKET_S_ADDR(local_sa.sin_addr)) ) {
            PPE_LOG_NETWORKERROR("inet_pton()");
            if (::closesocket(sockfd))
                PPE_LOG_NETWORKERROR("closesocket()");
            return false;
        }
    }

    // set the local port
    local_sa.sin_port = ::htons(checked_cast<::u_short>(_local.Port()));

    // bind the new socket to the requested local port and local ip
    if (_local.Port() != size_t(EServiceName::Any)) {
        if (SOCKET_ERROR == ::bind(sockfd, reinterpret_cast<sockaddr*>(&local_sa), sizeof(sockaddr_in)) ) {
            PPE_LOG_NETWORKERROR("bind()");
            if (::closesocket(sockfd))
                PPE_LOG_NETWORKERROR("closesocket()");
            return false;
        }
    }

    // connect the socket
    if (SOCKET_ERROR == ::connect(sockfd, reinterpret_cast<sockaddr*>(&foreign_sa), sizeof(sockaddr_in)) ) {
        PPE_LOG_NETWORKERROR("connect()");
        if (0 != ::closesocket(sockfd))
            PPE_LOG_NETWORKERROR("closesocket()");
        return false;
    }

    // determine the local port and IP and store them in used_local_ip
    // and used_local_port
    ::u_short used_local_port;
    ::sockaddr_in local_info;
    FString used_local_ip;
    if (_local.Port() == size_t(EServiceName::Any)) {
        ::socklen_t length = sizeof(::sockaddr_in);
        if (SOCKET_ERROR == ::getsockname(sockfd, reinterpret_cast<::sockaddr*>(&local_info), &length)) {
            PPE_LOG_NETWORKERROR("getsockname()");
            if (::closesocket(sockfd))
                PPE_LOG_NETWORKERROR("closesocket()");
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
            ::socklen_t length = sizeof(sockaddr_in);
            if (SOCKET_ERROR == ::getsockname(sockfd, reinterpret_cast<::sockaddr*>(&local_info), &length) ) {
                PPE_LOG_NETWORKERROR("getsockname()");
                if (::closesocket(sockfd))
                    PPE_LOG_NETWORKERROR("closesocket()");
                return false;
            }
        }

        char temp[INET6_ADDRSTRLEN];
        STATIC_ASSERT(INET_ADDRSTRLEN < INET6_ADDRSTRLEN); // should handle both ip categories
        const char* real_local_ip = ::inet_ntop(ai_family, &local_info.sin_addr, temp, lengthof(temp));

        // check if inet_ntop returned an error
        if (nullptr == real_local_ip) {
            PPE_LOG_NETWORKERROR("inet_ntop()");
            if (::closesocket(sockfd))
                PPE_LOG_NETWORKERROR("closesocket()");
            return false;
        }

        used_local_ip.assign(MakeCStringView(real_local_ip));
    }
    else {
        Assert(_local.IsIPv4());

        used_local_ip = _local.Host();
    }

    // Protocol Independent OOB Data
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ms740102(v=vs.85).aspx
    int flag_value = 1;
    if (SOCKET_ERROR == ::setsockopt(sockfd, SOL_SOCKET, SO_OOBINLINE, reinterpret_cast<const char*>(&flag_value), sizeof(int)) ) {
        PPE_LOG_NETWORKERROR("setsockopt()");
        if (::closesocket(sockfd))
            PPE_LOG_NETWORKERROR("closesocket()");
        return false;
    }

    // set timeout on socket
    SetTimeout(_timeout);

    _handle = PackSocket_(sockfd);
    _local = FAddress(std::move(used_local_ip), used_local_port);

    Assert(IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool FSocket::Disconnect(bool gracefully/* = false */) {
    Assert(IsConnected());

    SOCKET sockfd = UnpackSocket_(_handle);

    if (gracefully) {
        // shutdown every outgoing packets
        if (ShutdownOutgoing()) {
            // wait for the other end to close their side
            u8 junk[100];
            while (Read(MakeView(junk)) > 0);
        }
    }

    if (::closesocket(sockfd)) {
        PPE_LOG_NETWORKERROR("closesocket()");
        return false;
    }

    _handle = GInvalidSocket_;

    Assert(!IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool FSocket::DisableNagle() {
    Assert(IsConnected());

    int flag = 1;
    const int status = ::setsockopt(UnpackSocket_(_handle), IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));

    if (SOCKET_ERROR == status) {
        PPE_LOG_NETWORKERROR("setsockopt()");
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
bool FSocket::ShutdownOutgoing() {
    Assert(IsConnected());

    const int status = ::shutdown(UnpackSocket_(_handle),
#if defined(PLATFORM_WINDOWS)
        SD_SEND);
#else
        SHUT_WR);
#endif

    if (SOCKET_ERROR == status) {
        PPE_LOG_NETWORKERROR("shutdown()");
        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
bool FSocket::IsConnected() const {
    return (GInvalidSocket_ != _handle);
}
//----------------------------------------------------------------------------
bool FSocket::IsReadable(const FMilliseconds& timeout) const {
    Assert(IsConnected());
    Assert(timeout.Value() >= 0);

    if (timeout.Value() == 0)
        return true;

    ::fd_set read_set;
    // initialize read_set
    FD_ZERO(&read_set);

    // add the listening socket to read_set
    FD_SET(UnpackSocket_(_handle), &read_set);

    // setup a timeval structure
    ::timeval time_to_wait = MakeTimeval(timeout);

    // wait on select
    const int status = ::select(0, &read_set, 0, 0, &time_to_wait);

    // if select timed out
    if (status == 0)
        return false;

    // if select error
    if (SOCKET_ERROR == status) {
        PPE_LOG_NETWORKERROR("select()");
        return false;
    }

    // data is ready to be read
    return true;
}
//----------------------------------------------------------------------------
size_t FSocket::Read(const TMemoryView<u8>& rawData, bool block/* = true */) {
    Assert(!rawData.empty());
    Assert(IsConnected());

    // Make sure to cap the max value num can take on so that if it is
    // really large (it might be big on 64bit platforms) so that the OS
    // can't possibly get upset about it being large.
    const int length = (int)Min(MaxRecvLength_, rawData.size());

    const int flags = (block ? MSG_WAITALL : 0);
    const int status = ::recv(UnpackSocket_(_handle), (char*)rawData.data(), length, flags);

    if (SOCKET_ERROR == status) {
        PPE_LOG_NETWORKERROR("recv()");
        return 0;
    }
    else {
        return status;
    }
}
//----------------------------------------------------------------------------
size_t FSocket::Read(const TMemoryView<u8>& rawData, const FMilliseconds& timeout) {
    return (IsReadable(timeout) ? Read(rawData) : 0);
}
//----------------------------------------------------------------------------
size_t FSocket::Write(const TMemoryView<const u8>& rawData) {
    Assert(!rawData.empty());
    Assert(IsConnected());

    SOCKET sockfd = UnpackSocket_(_handle);

    size_t offset = 0;
    while (offset < rawData.size()) {
        // Make sure to cap the max value num can take on so that if it is
        // really large (it might be big on 64bit platforms) so that the OS
        // can't possibly get upset about it being large.
        const int length = (int)Min(MaxSendLength_, rawData.size() - offset);

        const int status = ::send(sockfd, (const char*)rawData.data() + offset, length, 0);
        if (SOCKET_ERROR == status) {
            PPE_LOG_NETWORKERROR("send()");
            return offset;
        }

        offset += checked_cast<size_t>(status);
    }

    return offset;
}
//----------------------------------------------------------------------------
bool FSocket::SetTimeout(const FMilliseconds& timeout) {
    Assert(timeout.Value() > 0);

    _timeout = timeout;

    if (IsConnected()) {
        SOCKET sockfd = UnpackSocket_(_handle);

#ifdef PLATFORM_WINDOWS
        // setup a DWORD in milliseconds
        ::DWORD time_to_wait = ::DWORD(_timeout.Value());

        // set timeout on socket
        if (SOCKET_ERROR == ::setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&time_to_wait, sizeof(time_to_wait)) ) {
            ::closesocket(sockfd);
            return false;
        }
#else
        // setup a timeval structure
        ::timeval time_to_wait = MakeTimeval(_timeout);

        // set timeout on socket
        if (SOCKET_ERROR == ::setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const void *)&time_to_wait, sizeof(time_to_wait)) ) {
            ::closesocket(sockfd);
            return false;
        }
#endif
    }

    return true;
}
//----------------------------------------------------------------------------
bool FSocket::MakeConnection(FSocket& socket, const FAddress& remoteHostnameOrIP) {
    Assert(!socket.IsConnected());
    Assert(!remoteHostnameOrIP.empty());

    socket._local = FAddress();

    if (remoteHostnameOrIP.IsIPv4()) {
        socket._remote = remoteHostnameOrIP;
    }
    else {
        FString ipv4;
        if (not HostnameToIPv4(ipv4, remoteHostnameOrIP.Host(), remoteHostnameOrIP.Port()) )
            return false;

        socket._remote = FAddress(std::move(ipv4), remoteHostnameOrIP.Port());
    }
    Assert(socket._remote.IsIPv4());

    return socket.Connect();
}
//----------------------------------------------------------------------------
void FSocket::Start() {
#ifdef PLATFORM_WINDOWS
    ::WSADATA wsaData;
    if (0 != ::WSAStartup(MAKEWORD(2,2), &wsaData))
        AssertNotReached();
#endif
}
//----------------------------------------------------------------------------
void FSocket::Shutdown() {
#ifdef PLATFORM_WINDOWS
    ::WSACleanup();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
