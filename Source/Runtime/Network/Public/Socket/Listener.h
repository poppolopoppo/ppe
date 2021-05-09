#pragma once

#include "Network_fwd.h"

#include "Socket/Address.h"

#include "Maths/Units.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FListener {
public:
    FListener() NOEXCEPT;
    explicit FListener(FAddress&& listening) NOEXCEPT;
    explicit FListener(const FAddress& listening)
        : FListener(FAddress(listening)) {}
    ~FListener();

    FListener(FListener&& rvalue) NOEXCEPT
        : FListener() {
        operator =(std::move(rvalue));
    }
    FListener& operator =(FListener&& rvalue) NOEXCEPT;

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
        explicit operator bool() const NOEXCEPT {
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
