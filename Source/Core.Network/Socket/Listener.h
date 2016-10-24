#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Socket/Address.h"

#include "Core/Maths/Units.h"

namespace Core {
namespace Network {
class FSocket;
class FSocketBuffered;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FListener {
public:
    FListener();
    explicit FListener(FAddress&& listening);
    explicit FListener(const FAddress& listening)
        : FListener(std::move(FAddress(listening))) {}
    ~FListener();

    FListener(FListener&& rvalue) : FListener() { operator =(std::move(rvalue)); }
    FListener& operator =(FListener&& rvalue);

    FListener(const FListener& ) = delete;
    FListener& operator =(const FListener& ) = delete;

    void* Handle() const { return _handle; }

    const FAddress& Listening() const { return _listening; }

    bool Connect();
    bool Disconnect();

    bool IsConnected() const;

    bool Accept(FSocket& socket, const Milliseconds& timeout);
    bool Accept(FSocketBuffered& socket, const Milliseconds& timeout);

    static FListener Localhost(size_t port);

    struct FConnectionScope {
        FListener& Listener;
        FConnectionScope(FListener& listener) : Listener(listener) {
            Listener.Connect();
        }
        ~FConnectionScope() {
            Listener.Disconnect();
        }
    };

private:
    void* _handle;

    FAddress _listening;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
