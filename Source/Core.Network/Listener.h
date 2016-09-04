#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Address.h"

#include "Core/Maths/Units.h"

namespace Core {
namespace Network {
class Socket;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Listener {
public:
    Listener();
    explicit Listener(Address&& listening);
    explicit Listener(const Address& listening)
        : Listener(std::move(Address(listening))) {}
    ~Listener();

    Listener(Listener&& rvalue) : Listener() { operator =(std::move(rvalue)); }
    Listener& operator =(Listener&& rvalue);

    Listener(const Listener& ) = delete;
    Listener& operator =(const Listener& ) = delete;

    void* Handle() const { return _handle; }

    const Address& Listening() const { return _listening; }

    bool Connect();
    bool Disconnect();

    bool IsConnected() const;

    bool Accept(Socket& socket, const Milliseconds& timeout);

private:
    void* _handle;

    Address _listening;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
