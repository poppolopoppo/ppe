#pragma once

#ifndef EXPORT_PPE_RUNTIME_NETWORK
#   error "This file should not be included outside of Core.Network"
#endif

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#if defined(PLATFORM_WINDOWS)

#   ifndef _MSC_VER
#       error "This source file is intended for Visual Studio"
#   endif

#   include <WinSock2.h>
#   include <Ws2tcpip.h>

#   ifndef _WINSOCKAPI_
#       error "_WINSOCKAPI_ must be defined to prevent inclusion of winsock.h"
#   endif

#   include "HAL/Windows/LastError.h"
#   include "HAL/Windows/WindowsPlatformIncludes.h"

using socklen_t = int;

#   define SOCKET_S_ADDR(_ADDR) ((_ADDR).S_un.S_addr)

#elif defined(PLATFORM_POSIX)

#   include "HAL/Linux/Errno.h"

#   include <arpa/inet.h>
#   include <netdb.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>

#   define SOCKET int
#   define SOCKET_ERROR (SOCKET(-1))
#   define SOCKET_S_ADDR(_ADDR) ((_ADDR).s_addr)
#   define INVALID_SOCKET (SOCKET(-1))

inline int closesocket(SOCKET sock) {
    return ::close(sock);
}

#else

#   error "#TODO: fix includes, but should be pretty close to the windows version (since M$ stole unix network layer)"

#endif

#include "Network.h"
#include "Diagnostic/Logger.h"

namespace PPE {
namespace Network {
EXTERN_LOG_CATEGORY(PPE_NETWORK_API, Network)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, MaxRecvLength_, 1024*1024*100);
STATIC_CONST_INTEGRAL(size_t, MaxSendLength_, 1024*1024*100);
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(SOCKET) <= sizeof(void*));
STATIC_ASSERT(Meta::is_pod_v<SOCKET>);
//----------------------------------------------------------------------------
constexpr intptr_t PackSocket_(SOCKET socket) {
    return static_cast<intptr_t>(socket);
}
//----------------------------------------------------------------------------
constexpr SOCKET UnpackSocket_(intptr_t handle) {
    STATIC_ASSERT(sizeof(SOCKET) <= sizeof(handle));
    return static_cast<SOCKET>(handle);
}
//----------------------------------------------------------------------------
#if defined(PLATFORM_WINDOWS)
#   define LOG_NETWORKERROR(_CONTEXT) \
        LOG(Network, Warning, _CONTEXT " failed, WSA last error : {0}", ::PPE::FLastError(::WSAGetLastError()))
#elif defined(PLATFORM_POSIX)
#   define LOG_NETWORKERROR(_CONTEXT) \
        LOG(Network, Warning, _CONTEXT " failed, socket last error : {0}", ::PPE::FErrno{})
#else
#   define LOG_NETWORKERROR(_CONTEXT) NOOP()
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
