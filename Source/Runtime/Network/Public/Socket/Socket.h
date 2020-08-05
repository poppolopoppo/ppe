#pragma once

#include "Network_fwd.h"

#include "Socket/Address.h"

#include "Maths/Units.h"
#include "Memory/MemoryView.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FSocket {
public:
    friend class FListener;

    STATIC_CONST_INTEGRAL(size_t, DefaultTimeoutInMs, 100);

    FSocket();
    FSocket(FAddress&& remote, FAddress&& local);
    FSocket(const FAddress& remote, const FAddress& local)
        : FSocket(FAddress(remote), FAddress(local)) {}
    explicit FSocket(FAddress&& remote)
        : FSocket(std::move(remote), FAddress()) {}
    explicit FSocket(const FAddress& remote)
        : FSocket(FAddress(remote), FAddress()) {}
    ~FSocket();

    FSocket(const FSocket& ) = delete;
    FSocket& operator =(const FSocket& ) = delete;

    FSocket(FSocket&& rvalue) : FSocket() { operator =(std::move(rvalue)); }
    FSocket& operator =(FSocket&& rvalue);

    intptr_t Handle() const { return _handle; }

    void* UserData() const { return _userData; }
    void SetUserData(void* ptr) { _userData = ptr; }

    const FAddress& Local() const { return _local; }
    const FAddress& Remote() const { return _remote; }

    const FMilliseconds& Timeout() const { return _timeout; }
    bool SetTimeout(const FMilliseconds& timeout);

    bool Connect();
    bool Disconnect(bool gracefully = false);

    // Turning of Nagle might reduce latency, but is not recommended
    // https://fr.wikipedia.org/wiki/Algorithme_de_Nagle
    bool DisableNagle();
    bool ShutdownOutgoing();

    bool IsConnected() const;
    bool IsReadable(const FMilliseconds& timeout) const;

    size_t Read(const TMemoryView<u8>& rawData, bool block = true);
    size_t Read(const TMemoryView<u8>& rawData, const FMilliseconds& timeout);
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
    intptr_t _handle;
    void* _userData;

    FAddress _local;
    FAddress _remote;

    FMilliseconds _timeout;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
