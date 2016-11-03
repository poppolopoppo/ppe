#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Name.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHttpConstNames {
private:
    FHttpConstNames();

public:
    // Http headers
    static const FName& Accept();
    static const FName& AcceptCharset();
    static const FName& AcceptEncoding();
    static const FName& CacheControl();
    static const FName& Connection();
    static const FName& Cookie();
    static const FName& ContentLanguage();
    static const FName& ContentLength();
    static const FName& ContentType();
    static const FName& Date();
    static const FName& Location();
    static const FName& Referer();
    static const FName& RetryAfter();
    static const FName& Server();
    static const FName& Status();
    static const FName& UserAgent();

    static void Start();
    static void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
