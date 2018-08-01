#include "stdafx.h"

#include "Listener.h"

#include "NetworkIncludes.h"
#include "Socket.h"
#include "SocketBuffered.h"

#include "Core/Diagnostic/Logger.h"

// FSocket implementation adapted/updated from C++ DLib
// http://dlib.net/files/dlib-19.1.zip
// http://dlib.net/dlib/sockets/sockets_kernel_1.h.html

namespace Core {
namespace Network {
EXTERN_LOG_CATEGORY(CORE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FListener::FListener() : _handle(0) {}
//----------------------------------------------------------------------------
FListener::FListener(FAddress&& listening)
:   _handle(0)
,   _listening(std::move(listening)) {
    Assert(_listening.IsIPv4());
}
//----------------------------------------------------------------------------
FListener::~FListener() {
    Assert(!IsConnected());
}
//----------------------------------------------------------------------------
FListener& FListener::operator =(FListener&& rvalue) {
    if (IsConnected())
        Disconnect();

    _handle = rvalue._handle;
    _listening = std::move(rvalue._listening);

    rvalue._handle = 0;

    return *this;
}
//----------------------------------------------------------------------------
bool FListener::Connect() {
    Assert(!IsConnected());

    ::sockaddr_in sa;  // local socket structure
    ZeroMemory(&sa, sizeof(::sockaddr_in)); // initialize sa

    ::SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);  // get a new socket

    // if socket() returned an error then return OTHER_ERROR
    if (INVALID_SOCKET == sock) {
        LOG_WSALASTERROR(L"socket()");
        return false;
    }

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
            LOG_WSALASTERROR(L"inet_pton()");
            if (::closesocket(sock))
                LOG_WSALASTERROR(L"closesocket()");
            return false;
        }
    }

    // set the SO_REUSEADDR option
    int flag_value = 1;
    if (SOCKET_ERROR == ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&flag_value), sizeof(int))) {
        LOG_WSALASTERROR(L"setsockopt()");
        if (::closesocket(sock))
            LOG_WSALASTERROR(L"closesocket()");
    }

    // bind the new socket to the requested port and ip
    if (SOCKET_ERROR == ::bind(sock, reinterpret_cast<sockaddr*>(&sa), sizeof(::sockaddr_in)) ) {
        LOG_WSALASTERROR(L"bind()");
        if (::closesocket(sock))
            LOG_WSALASTERROR(L"closesocket()");
        return false;
    }

    // tell the new socket to listen
    if (SOCKET_ERROR == ::listen(sock, SOMAXCONN)) {
        LOG_WSALASTERROR(L"listen()");
        if (::closesocket(sock))
            LOG_WSALASTERROR(L"closesocket()");
        return false;
    }

    // determine the port used if necessary
    if (_listening.Port() == 0) {
        ::sockaddr_in local_info;
        int length = sizeof(::sockaddr_in);

        if (SOCKET_ERROR == ::getsockname (sock, reinterpret_cast<sockaddr*>(&local_info), &length) ) {
            LOG_WSALASTERROR(L"getsockname()");
            if (::closesocket(sock))
                LOG_WSALASTERROR(L"closesocket()");
            return false;
        }

        const ::u_short port = ::ntohs(local_info.sin_port);
        _listening.SetPort(port);
    }

    _handle = PackSocket_(sock);

    LOG(Network, Info, L"started listening to {0}", _listening);

    Assert(IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool FListener::Disconnect() {
    Assert(IsConnected());

    ::SOCKET sock = UnpackSocket_(_handle);
    _handle = 0;

    const int status = ::closesocket(sock);
    if (status == -1)
        return false;

    LOG(Network, Info, L"stopped listening to {0}", _listening);

    Assert(!IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool FListener::IsConnected() const {
    return (0 != _handle && INVALID_SOCKET != UnpackSocket_(_handle));
}
//----------------------------------------------------------------------------
bool FListener::Accept(FSocket& socket, const FMilliseconds& timeout) {
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
        if (SOCKET_ERROR == status) {
            LOG_WSALASTERROR(L"select()");
            return false;
        }
    }

    // call accept to get a new connection
    incoming = ::accept(UnpackSocket_(_handle), reinterpret_cast<::sockaddr*>(&incomingAddr), &length);

    // if there was an error return OTHER_ERROR
    if (INVALID_SOCKET == incoming) {
        LOG_WSALASTERROR(L"accept()");
        return false;
    }

    // get the port of the foreign host into foreign_port
    int foreign_port = ::ntohs(incomingAddr.sin_port);

    // get the IP of the foreign host into foreign_ip
    FString foreign_ip;
    {
        char temp[17];
        const char* real_foreign_ip = ::inet_ntop(AF_INET, &incomingAddr.sin_addr, temp, lengthof(temp));

        // check if inet_ntop() returned an error
        if (nullptr == real_foreign_ip) {
            LOG_WSALASTERROR(L"inet_ntop()");
            if (::closesocket(incoming))
                LOG_WSALASTERROR(L"closesocket()");
            return false;
        }

        foreign_ip.assign(MakeCStringView(real_foreign_ip));
    }

    // get the local ip
    FString local_ip;
    if (_listening.empty()) {
        ::sockaddr_in local_info;
        length = sizeof(::sockaddr_in);

        // get the local sockaddr_in structure associated with this new connection
        if (SOCKET_ERROR == ::getsockname(incoming, reinterpret_cast<::sockaddr*>(&local_info), &length)) {
            LOG_WSALASTERROR(L"getsockname()");
            if (::closesocket(incoming))
                LOG_WSALASTERROR(L"closesocket()");
            return false;
        }

        char temp[17];
        const char* real_local_ip = ::inet_ntop(AF_INET, &local_info.sin_addr, temp, lengthof(temp));

        // check if inet_ntop() returned an error
        if (nullptr == real_local_ip) {
            LOG_WSALASTERROR(L"inet_ntop()");
            if (::closesocket(incoming))
                LOG_WSALASTERROR(L"closesocket()");
            return false;
        }

        local_ip.assign(MakeCStringView(real_local_ip));
    }
    else {
        local_ip = _listening.Host();
    }

    // set the SO_OOBINLINE option
    int flag_value = 1;
    if (SOCKET_ERROR == ::setsockopt(incoming, SOL_SOCKET, SO_OOBINLINE, reinterpret_cast<const char*>(&flag_value), sizeof(int)) ) {
        LOG_WSALASTERROR(L"setsockopt()");
        if (::closesocket(incoming))
            LOG_WSALASTERROR(L"closesocket()");
        return false;
    }

    socket._handle = PackSocket_(incoming);
    socket._local.SetHost(std::move(local_ip));
    socket._local.SetPort(_listening.Port());
    socket._remote.SetHost(std::move(foreign_ip));
    socket._remote.SetPort(foreign_port);

    socket.SetTimeout(socket.Timeout()); // force set the SO_RCVTIMEO socket option

    LOG(Network, Info, L"accept socket from {0} to {1}", socket._local, socket._remote );

    Assert(socket.IsConnected());
    return true;
}
//----------------------------------------------------------------------------
bool FListener::Accept(FSocketBuffered& socket, const FMilliseconds& timeout) {
    return FSocketBuffered::Accept(socket, *this, timeout);
}
//----------------------------------------------------------------------------
FListener FListener::Localhost(size_t port) {
    return FListener(FAddress::Localhost(port));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
