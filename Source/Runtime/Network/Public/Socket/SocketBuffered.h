#pragma once

#include "Network_fwd.h"

#include "Socket/Socket.h"

#include "IO/TextWriter_fwd.h"
#include "Memory/SharedBuffer.h"
#include "Memory/UniqueView.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSocketBuffered {
public:
    STATIC_CONST_INTEGRAL(size_t, DefaultBufferSize, 2048);

    FSocketBuffered() NOEXCEPT = default;
    ~FSocketBuffered() = default;

    PPE_NETWORK_API explicit FSocketBuffered(FSocket&& socket, size_t capacity = DefaultBufferSize);

    FSocketBuffered(const FSocketBuffered& ) = delete;
    FSocketBuffered& operator =(const FSocketBuffered& ) = delete;

    PPE_NETWORK_API FSocketBuffered(FSocketBuffered&& rvalue) NOEXCEPT;
    PPE_NETWORK_API FSocketBuffered& operator =(FSocketBuffered&& rvalue) NOEXCEPT;

    NODISCARD const FSocket& Socket() const { return _socket; }

    NODISCARD intptr_t Handle() const { return _socket.Handle(); }

    NODISCARD void* UserData() const { return _socket.UserData(); }
    void SetUserData(void* ptr) { _socket.SetUserData(ptr); }

    NODISCARD const FAddress& Local() const { return _socket.Local(); }
    NODISCARD const FAddress& Remote() const { return _socket.Remote(); }

    NODISCARD const FMilliseconds& Timeout() const { return _socket.Timeout(); }
    NODISCARD bool SetTimeout(const FMilliseconds& timeout) { return _socket.SetTimeout(timeout); }

    NODISCARD PPE_NETWORK_API bool Connect();
    PPE_NETWORK_API bool Disconnect(bool gracefully = false);
    PPE_NETWORK_API bool ShutdownOutgoing();

    NODISCARD PPE_NETWORK_API bool IsConnected() const;
    NODISCARD PPE_NETWORK_API bool IsReadable() const;
    NODISCARD PPE_NETWORK_API bool IsReadable(const FMilliseconds& timeout) const;

    NODISCARD PPE_NETWORK_API size_t Read(const TMemoryView<u8>& rawData);

    NODISCARD PPE_NETWORK_API size_t Write(const TMemoryView<const u8>& rawData);
    NODISCARD PPE_NETWORK_API size_t Write(const FStringView& str) { return Write(str.Cast<const u8>()); }
    NODISCARD PPE_NETWORK_API size_t Write(FStringLiteral literal) { return Write(literal.MakeView()); }

    template <typename T>
    NODISCARD bool ReadPOD(T& assumePOD);
    template <typename T>
    NODISCARD bool PeekPOD(T& assumePOD);
    template <typename T>
    NODISCARD bool WritePOD(const T& assumePOD);

    NODISCARD bool Get(char& ch) { return ReadPOD(ch); }
    NODISCARD bool Peek(char& ch) { return PeekPOD(ch); }
    NODISCARD bool Put(char ch) { return WritePOD(ch); }

    PPE_NETWORK_API void EatWhiteSpaces();
    NODISCARD PPE_NETWORK_API bool ReadUntil(FTextWriter* poss, char delim);

    PPE_NETWORK_API void FlushRead(bool block = false);
    PPE_NETWORK_API bool FlushWrite();

    NODISCARD PPE_NETWORK_API static bool Accept(FSocketBuffered& buffered, FListener& listener, const FMilliseconds& timeout);
    NODISCARD PPE_NETWORK_API static bool MakeConnection(FSocketBuffered& buffered, const FAddress& remoteHostnameOrIP);

private:
    size_t ReadFromBuffer_(const TMemoryView<u8>& rawData);

    FSocket _socket;

    FUniqueBuffer _bufferI;
    FUniqueBuffer _bufferO;

    size_t _sizeI{0};
    size_t _offsetI{0};
    size_t _sizeO{0};
    size_t _bufferCapacity{DefaultBufferSize};
};
//----------------------------------------------------------------------------
template <typename T>
bool FSocketBuffered::ReadPOD(T& assumePOD) {
    return (sizeof(T) == Read(MakePodView(assumePOD)) );
}
//----------------------------------------------------------------------------
template <typename T>
bool FSocketBuffered::PeekPOD(T& assumePOD) {
    if (_offsetI + sizeof(T) > _sizeI)
        FlushRead();

    if (_offsetI + sizeof(T) <= _sizeI) {
        _bufferI.MakeView().CutStartingAt(_offsetI).EatRaw(&assumePOD);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename T>
bool FSocketBuffered::WritePOD(const T& assumePOD) {
    return (sizeof(T) == Write(MakePodView(assumePOD)) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
