#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Address.h"

#include "Core/Maths/Units.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Socket {
public:
    friend class Listener;

    Socket();
    Socket(Address&& remote, Address&& local);
    Socket(const Address& remote, const Address& local)
        : Socket(std::move(Address(remote)), std::move(Address(local))) {}
    explicit Socket(Address&& remote)
        : Socket(std::move(remote), Address()) {}
    explicit Socket(const Address& remote)
        : Socket(std::move(Address(remote)), Address()) {}
    ~Socket();

    Socket(const Socket& ) = delete;
    Socket& operator =(const Socket& ) = delete;

    Socket(Socket&& rvalue) : Socket() { operator =(std::move(rvalue)); }
    Socket& operator =(Socket&& rvalue);

    void* Handle() const { return _handle; }

    void* UserData() const { return _userData; }
    void SetUserData(void* ptr) { _userData = ptr; }

    const Address& Local() const { return _local; }
    const Address& Remote() const { return _remote; }

    bool Connect();
    bool Disconnect(bool gracefully = false);

    // Turning of Nagle might reduce latency, but is not recommended
    // https://fr.wikipedia.org/wiki/Algorithme_de_Nagle
    bool DisableNagle();
    bool ShutdownOutgoing();

    bool IsConnected() const;
    bool IsReadable(const Milliseconds& timeout) const;

    size_t Read(MemoryView<u8>& rawData);
    size_t Read(MemoryView<u8>& rawData, const Milliseconds& timeout);
    size_t Write(const MemoryView<const u8>& rawData);

    static void Start();
    static void Shutdown();

    static bool MakeConnection(Socket& socket, const Address& remoteHostnameOrIP);

private:
    void* _handle;
    void* _userData;

    Address _local;
    Address _remote;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
