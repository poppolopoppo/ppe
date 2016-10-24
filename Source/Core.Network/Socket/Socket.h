#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Socket/Address.h"

#include "Core/Maths/Units.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSocket {
public:
    friend class FListener;

    FSocket();
    FSocket(FAddress&& remote, FAddress&& local);
    FSocket(const FAddress& remote, const FAddress& local)
        : FSocket(std::move(FAddress(remote)), std::move(FAddress(local))) {}
    explicit FSocket(FAddress&& remote)
        : FSocket(std::move(remote), FAddress()) {}
    explicit FSocket(const FAddress& remote)
        : FSocket(std::move(FAddress(remote)), FAddress()) {}
    ~FSocket();

    FSocket(const FSocket& ) = delete;
    FSocket& operator =(const FSocket& ) = delete;

    FSocket(FSocket&& rvalue) : FSocket() { operator =(std::move(rvalue)); }
    FSocket& operator =(FSocket&& rvalue);

    void* Handle() const { return _handle; }

    void* UserData() const { return _userData; }
    void SetUserData(void* ptr) { _userData = ptr; }

    const FAddress& Local() const { return _local; }
    const FAddress& Remote() const { return _remote; }

    bool Connect();
    bool Disconnect(bool gracefully = false);

    // Turning of Nagle might reduce latency, but is not recommended
    // https://fr.wikipedia.org/wiki/Algorithme_de_Nagle
    bool DisableNagle();
    bool ShutdownOutgoing();

    bool IsConnected() const;
    bool IsReadable(const Milliseconds& timeout) const;

    size_t Read(const TMemoryView<u8>& rawData, bool block = false);
    size_t Read(const TMemoryView<u8>& rawData, const Milliseconds& timeout);
    size_t Write(const TMemoryView<const u8>& rawData);

    static void Start();
    static void Shutdown();

    static bool MakeConnection(FSocket& socket, const FAddress& remoteHostnameOrIP);

    struct FConnectionScope {
        FSocket& Socket;
        FConnectionScope(FSocket& socket) : Socket(socket) {
            Socket.Connect();
        }
        ~FConnectionScope() {
            Socket.Disconnect(true);
        }
    };

private:
    void* _handle;
    void* _userData;

    FAddress _local;
    FAddress _remote;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
