#pragma once

#include "Core.Network/Network.h"

#include "Core.Network/Socket/Socket.h"

#include "Core/Memory/UniqueView.h"

namespace Core {
namespace Network {
class FListener;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSocketBuffered {
public:
    STATIC_CONST_INTEGRAL(size_t, DefaultBufferSize, 2048);

    FSocketBuffered();
    explicit FSocketBuffered(FSocket&& socket, size_t capacity = DefaultBufferSize);
    ~FSocketBuffered();

    FSocketBuffered(const FSocketBuffered& ) = delete;
    FSocketBuffered& operator =(const FSocketBuffered& ) = delete;

    FSocketBuffered(FSocketBuffered&& rvalue);
    FSocketBuffered& operator =(FSocketBuffered&& rvalue);

    const FSocket& Socket() const { return _socket; }

    bool Connect();
    bool Disconnect(bool gracefully = false);
    bool ShutdownOutgoing();

    bool IsConnected() const;
    bool IsReadable(const Milliseconds& timeout) const;

    size_t Read(const TMemoryView<u8>& rawData);
    size_t Read(const TMemoryView<u8>& rawData, const Milliseconds& timeout);
    size_t Write(const TMemoryView<const u8>& rawData);

    void FlushRead();
    void FlushRead(const Milliseconds& timeout);
    void FlushWrite();

    static bool Accept(FSocketBuffered& buffered, FListener& listener, const Milliseconds& timeout);
    static bool MakeConnection(FSocketBuffered& buffered, const FAddress& remoteHostnameOrIP);

private:
    size_t ReadFromBuffer_(const TMemoryView<u8>& rawData);

    FSocket _socket;

    size_t _sizeI;
    size_t _offsetI;
    TUniqueArray<u8> _bufferI;

    size_t _sizeO;
    TUniqueArray<u8> _bufferO;

    size_t _bufferCapacity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
