#pragma once

#ifndef EXPORT_PPE_NETWORK
#   error "This file should not be included outside of Core.Network"
#endif

#ifdef PLATFORM_WINDOWS

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
STATIC_ASSERT(sizeof(::SOCKET) <= sizeof(void*));
STATIC_ASSERT(Meta::TIsPod<::SOCKET>::value);
//----------------------------------------------------------------------------
constexpr intptr_t PackSocket_(::SOCKET socket) {
    return static_cast<intptr_t>(socket);
}
//----------------------------------------------------------------------------
constexpr ::SOCKET UnpackSocket_(intptr_t handle) {
    STATIC_ASSERT(sizeof(::SOCKET) <= sizeof(handle));
    return static_cast<::SOCKET>(handle);
}
//----------------------------------------------------------------------------
#if defined(USE_DEBUG_LOGGER) && defined(PLATFORM_WINDOWS)
#   define LOG_WSALASTERROR(_CONTEXT) \
        LOG(Network, Error, _CONTEXT "failed, WSA last error : {0}", ::PPE::FLastError(::WSAGetLastError()))
#else
#   define LOG_WSALASTERROR(_CONTEXT) NOOP()
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
