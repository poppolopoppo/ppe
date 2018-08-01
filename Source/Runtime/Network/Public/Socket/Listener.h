#pragma once

#include "Network.h"

#include "Socket/Address.h"

#include "Maths/Units.h"

namespace PPE {
namespace Network {
class FSocket;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FListener {
public:
    FListener();
    explicit FListener(FAddress&& listening);
    explicit FListener(const FAddress& listening)
        : FListener(FAddress(listening)) {}
    ~FListener();

    FListener(FListener&& rvalue) : FListener() { operator =(std::move(rvalue)); }
    FListener& operator =(FListener&& rvalue);

    FListener(const FListener& ) = delete;
    FListener& operator =(const FListener& ) = delete;

    intptr_t Handle() const { return _handle; }

    const FAddress& Listening() const { return _listening; }

    bool Connect();
    bool Disconnect();

    bool IsConnected() const;

    bool Accept(FSocket& socket, const FMilliseconds& timeout);
    bool Accept(FSocketBuffered& socket, const FMilliseconds& timeout);

    static FListener Localhost(size_t port);

    struct FConnectionScope {
        FListener& Listener;
        bool Succeed;
        FConnectionScope(FListener& listener, size_t retries = 3) : Listener(listener) {
            do {
                Succeed = Listener.Connect();
            } while (not Succeed && retries--);
        }
        ~FConnectionScope() {
            if (Succeed)
                Listener.Disconnect();
        }
        operator bool() const {
            return Succeed;
        }
    };

private:
    intptr_t _handle;

    FAddress _listening;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
