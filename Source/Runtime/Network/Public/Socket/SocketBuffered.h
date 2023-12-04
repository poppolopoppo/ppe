#pragma once

#include "Network_fwd.h"

#include "Socket/Socket.h"

#include "IO/TextWriter_fwd.h"
#include "Memory/UniqueView.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_NETWORK_API FSocketBuffered {
public:
    STATIC_CONST_INTEGRAL(size_t, DefaultBufferSize, 2048);

    FSocketBuffered();
    explicit FSocketBuffered(FSocket&& socket, size_t capacity = DefaultBufferSize);
    ~FSocketBuffered();

    FSocketBuffered(const FSocketBuffered& ) = delete;
    FSocketBuffered& operator =(const FSocketBuffered& ) = delete;

    FSocketBuffered(FSocketBuffered&& rvalue) NOEXCEPT;
    FSocketBuffered& operator =(FSocketBuffered&& rvalue) NOEXCEPT;

    const FSocket& Socket() const { return _socket; }

    intptr_t Handle() const { return _socket.Handle(); }

    void* UserData() const { return _socket.UserData(); }
    void SetUserData(void* ptr) { _socket.SetUserData(ptr); }

    const FAddress& Local() const { return _socket.Local(); }
    const FAddress& Remote() const { return _socket.Remote(); }

    const FMilliseconds& Timeout() const { return _socket.Timeout(); }
    bool SetTimeout(const FMilliseconds& timeout) { return _socket.SetTimeout(timeout); }

    bool Connect();
    bool Disconnect(bool gracefully = false);
    bool ShutdownOutgoing();

    bool IsConnected() const;
    bool IsReadable() const;
    bool IsReadable(const FMilliseconds& timeout) const;

    size_t Read(const TMemoryView<u8>& rawData);

    size_t Write(const TMemoryView<const u8>& rawData);
    size_t Write(const FStringView& str) { return Write(str.Cast<const u8>()); }

    template <typename T> bool ReadPOD(T& assumePOD);
    template <typename T> bool PeekPOD(T& assumePOD);
    template <typename T> bool WritePOD(const T& assumePOD);

    bool Get(char& ch) { return ReadPOD(ch); }
    bool Peek(char& ch) { return PeekPOD(ch); }
    bool Put(char ch) { return WritePOD(ch); }

    void EatWhiteSpaces();
    bool ReadUntil(FTextWriter* poss, char delim);

    void FlushRead(bool block = false);
    bool FlushWrite();

    static bool Accept(FSocketBuffered& buffered, FListener& listener, const FMilliseconds& timeout);
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
        assumePOD = *reinterpret_cast<T*>(&_bufferI[_offsetI]);
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
