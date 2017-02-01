#include "stdafx.h"

#include "Socket.h"

#include "NetworkIncludes.h"

// FSocket implementation adapted/updated from C++ DLib
// http://dlib.net/files/dlib-19.1.zip
// http://dlib.net/dlib/sockets/sockets_kernel_1.h.html

namespace Core {
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
static constexpr intptr_t gInvalidSocket_ = PackSocket_(INVALID_SOCKET);
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSocket::FSocket()
:   _handle(gInvalidSocket_)
,   _userData(nullptr)
,   _timeout(double(DefaultTimeoutInMs)) {}
//----------------------------------------------------------------------------
FSocket::FSocket(FAddress&& remote, FAddress&& local)
:   _handle(gInvalidSocket_)
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
FSocket& FSocket::operator =(FSocket&& rvalue) {
    if (IsConnected())
        Disconnect();

    _handle = rvalue._handle;
    _local = std::move(rvalue._local);
    _remote = std::move(rvalue._remote);
    _timeout = std::move(rvalue._timeout);

    rvalue._handle = gInvalidSocket_;

    return *this;
}
//----------------------------------------------------------------------------
bool FSocket::Connect() {
    Assert(!IsConnected());
    Assert(!_remote.empty());

    ::sockaddr_in local_sa;     // local socket structure
    ::sockaddr_in foreign_sa;   // foreign socket structure
    ::ZeroMemory(&local_sa, sizeof(sockaddr_in));   // initialize local_sa
    ::ZeroMemory(&foreign_sa, sizeof(sockaddr_in)); // initialize foreign_sa

    ::SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, 0);  // get a new socket

    // if socket() returned an error then return OTHER_ERROR
    if (INVALID_SOCKET == sockfd)
        return false;

    // set the foreign socket structure
    foreign_sa.sin_family = AF_INET;
    foreign_sa.sin_port = ::htons(checked_cast<::u_short>(_remote.Port()));

    // if inet_pton couldn't convert the ip then return an error
    if (1 != ::inet_pton(AF_INET, _remote.Host().c_str(), &foreign_sa.sin_addr.S_un.S_addr) ) {
        ::closesocket(sockfd);
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
            ::closesocket(sockfd);
            return false;
        }
    }

    // set the local port
    local_sa.sin_port = ::htons(checked_cast<::u_short>(_local.Port()));

    // bind the new socket to the requested local port and local ip
    if (SOCKET_ERROR == ::bind(sockfd, reinterpret_cast<sockaddr*>(&local_sa), sizeof(sockaddr_in)) ) {
        ::closesocket(sockfd);
        return false;
    }

    // connect the socket
    if (SOCKET_ERROR == ::connect(sockfd, reinterpret_cast<sockaddr*>(&foreign_sa), sizeof(sockaddr_in)) ) {
        ::closesocket(sockfd);
        return false;
    }

    // determine the local port and IP and store them in used_local_ip
    // and used_local_port
    ::u_short used_local_port;
    ::sockaddr_in local_info;
    FString used_local_ip;
    if (0 == _local.Port()) {
        int length = sizeof(::sockaddr_in);
        if (SOCKET_ERROR == ::getsockname(sockfd, reinterpret_cast<::sockaddr*>(&local_info), &length)) {
            ::closesocket(sockfd);
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
            if (SOCKET_ERROR == ::getsockname(sockfd, reinterpret_cast<::sockaddr*>(&local_info), &length) ) {
                ::closesocket(sockfd);
                return false;
            }
        }

        char temp[17];
        const char* real_local_ip = ::inet_ntop(AF_INET, &local_info.sin_addr, temp, lengthof(temp));

        // check if inet_ntop returned an error
        if (nullptr == real_local_ip) {
            ::closesocket(sockfd);
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
    if (SOCKET_ERROR == ::setsockopt(sockfd, SOL_SOCKET, SO_OOBINLINE, reinterpret_cast<const char*>(&flag_value), sizeof(int)) ) {
        ::closesocket(sockfd);
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

    ::SOCKET sockfd = UnpackSocket_(_handle);

    if (gracefully) {
        // shutdown every outgoing packets
        if (ShutdownOutgoing()) {
            // wait for the other end to close their side
            u8 junk[100];
            while (Read(MakeView(junk)) > 0);
        }
    }

    const int status = ::closesocket(sockfd);
    if (status == -1)
        return false;

    _handle = gInvalidSocket_;

    Assert(!IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool FSocket::DisableNagle() {
    Assert(IsConnected());

    int flag = 1;
    const int status = ::setsockopt(UnpackSocket_(_handle), IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));

    return (SOCKET_ERROR != status);
}
//----------------------------------------------------------------------------
bool FSocket::ShutdownOutgoing() {
    Assert(IsConnected());

    const int status = ::shutdown(UnpackSocket_(_handle), SD_SEND);

    return (status != -1);
}
//----------------------------------------------------------------------------
bool FSocket::IsConnected() const {
    return (gInvalidSocket_ != _handle);
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

    // if select timed out or there was an error
    if (status <= 0)
        return false;

    // data is ready to be read
    return true;
}
//----------------------------------------------------------------------------
size_t FSocket::Read(const TMemoryView<u8>& rawData, bool block/* = false */) {
    Assert(!rawData.empty());
    Assert(IsConnected());

    // Make sure to cap the max value num can take on so that if it is
    // really large (it might be big on 64bit platforms) so that the OS
    // can't possibly get upset about it being large.
    const int length = (int)Min(MaxRecvLength_, rawData.size());

    const int flags = (block ? MSG_WAITALL : 0);
    const int status = ::recv(UnpackSocket_(_handle), (char*)rawData.data(), length, flags);

    return (SOCKET_ERROR == status ? 0 : status);
}
//----------------------------------------------------------------------------
size_t FSocket::Read(const TMemoryView<u8>& rawData, const FMilliseconds& timeout) {
    return (IsReadable(timeout) ? Read(rawData) : 0);
}
//----------------------------------------------------------------------------
size_t FSocket::Write(const TMemoryView<const u8>& rawData) {
    Assert(!rawData.empty());
    Assert(IsConnected());

    ::SOCKET sockfd = UnpackSocket_(_handle);

    size_t offset = 0;
    while (offset < rawData.size()) {
        // Make sure to cap the max value num can take on so that if it is
        // really large (it might be big on 64bit platforms) so that the OS
        // can't possibly get upset about it being large.
        const int length = (int)Min(MaxSendLength_, rawData.size() - offset);

        const int status = ::send(sockfd, (const char*)rawData.data() + offset, length, 0);
        if (SOCKET_ERROR == status)
            return offset;

        offset += checked_cast<size_t>(status);
    }

    return offset;
}
//----------------------------------------------------------------------------
bool FSocket::SetTimeout(const FMilliseconds& timeout) {
    Assert(timeout.Value() > 0);

    _timeout = timeout;

    if (IsConnected()) {
        ::SOCKET sockfd = UnpackSocket_(_handle);

#ifdef OS_WINDOWS
        // setup a DWORD in milliseconds
        DWORD time_to_wait = DWORD(_timeout.Value());

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
        FString ip;
        if (not HostnameToIP(ip, MakeView(remoteHostnameOrIP.Host())) )
            return false;

        socket._remote = FAddress(std::move(ip), remoteHostnameOrIP.Port());
    }
    Assert(socket._remote.IsIPv4());

    return socket.Connect();
}
//----------------------------------------------------------------------------
void FSocket::Start() {
#ifdef OS_WINDOWS
    ::WSADATA wsaData;
    if (0 != ::WSAStartup(MAKEWORD(2,2), &wsaData))
        AssertNotReached();
#endif
}
//----------------------------------------------------------------------------
void FSocket::Shutdown() {
#ifdef OS_WINDOWS
    ::WSACleanup();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
