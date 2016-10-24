#include "stdafx.h"

#include "SocketBuffered.h"

#include "Listener.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSocketBuffered::FSocketBuffered()
:   _sizeI(0)
,   _offsetI(0)
,   _sizeO(0)
,   _bufferCapacity(DefaultBufferSize) {}
//----------------------------------------------------------------------------
FSocketBuffered::FSocketBuffered(FSocket&& socket, size_t capacity/* = DefaultBufferSize */)
:   _socket(std::move(socket))
,   _sizeI(0)
,   _offsetI(0)
,   _sizeO(0)
,   _bufferCapacity(capacity) {
    Assert(_bufferCapacity > 0);
}
//----------------------------------------------------------------------------
FSocketBuffered::~FSocketBuffered() {}
//----------------------------------------------------------------------------
FSocketBuffered::FSocketBuffered(FSocketBuffered&& rvalue)
:   _socket(std::move(rvalue._socket))
,   _bufferI(std::move(rvalue._bufferI))
,   _bufferO(std::move(rvalue._bufferO)) {
}
//----------------------------------------------------------------------------
FSocketBuffered& FSocketBuffered::operator =(FSocketBuffered&& rvalue) {
    if (IsConnected())
        Disconnect(true);

    _socket = std::move(rvalue._socket);

    _sizeI = rvalue._sizeI;
    _offsetI = rvalue._offsetI;
    _bufferI = std::move(rvalue._bufferI);

    _sizeO = rvalue._sizeO;
    _bufferO = std::move(rvalue._bufferO);

    _bufferCapacity = rvalue._bufferCapacity;

    rvalue._sizeI = rvalue._offsetI = rvalue._sizeO = 0;

    Assert(_bufferI.empty() || _bufferI.SizeInBytes() == _bufferCapacity);
    Assert(_bufferO.empty() || _bufferO.SizeInBytes() == _bufferCapacity);

    return (*this);
}
//----------------------------------------------------------------------------
bool FSocketBuffered::Connect() {
    return _socket.Connect();
}
//----------------------------------------------------------------------------
bool FSocketBuffered::Disconnect(bool gracefully/* = false */) {
    FlushWrite();

    return _socket.Disconnect(gracefully);
}
//----------------------------------------------------------------------------
bool FSocketBuffered::ShutdownOutgoing() {
    FlushWrite();

    return _socket.ShutdownOutgoing();
}
//----------------------------------------------------------------------------
bool FSocketBuffered::IsConnected() const {
    return _socket.IsConnected();
}
//----------------------------------------------------------------------------
bool FSocketBuffered::IsReadable(const Milliseconds& timeout) const {
    return (not _bufferI.empty() || _socket.IsReadable(timeout));
}
//----------------------------------------------------------------------------
size_t FSocketBuffered::Read(const TMemoryView<u8>& rawData) {
    return Read(rawData, Milliseconds(0)/* no timeout, no wait */);
}
//----------------------------------------------------------------------------
size_t FSocketBuffered::Read(const TMemoryView<u8>& rawData, const Milliseconds& timeout) {
    Assert(rawData.size());

    size_t read = 0;

    if (_offsetI < _sizeI)
        read += ReadFromBuffer_(rawData);

    if (rawData.size() != read) // if there is still some data to read
    {
        Assert(_sizeI == _offsetI); // the buffer must be empty

        if (rawData.size() - read > _bufferCapacity)
        {
            // fallback to unbuffered read when we query a block larger than buffer capacity
            read += _socket.Read(rawData.CutStartingAt(read), timeout);
        }
        else
        {
            FlushRead(timeout); // refill the buffer
            read += ReadFromBuffer_(rawData);
        }
    }

    return read;
}
//----------------------------------------------------------------------------
size_t FSocketBuffered::Write(const TMemoryView<const u8>& rawData) {
    Assert(rawData.size());

    const size_t toWrite = rawData.SizeInBytes();

    if (toWrite > _bufferCapacity) {
        FlushWrite();

        return _socket.Write(rawData);
    }
    else {
        if (toWrite > _bufferCapacity - _sizeO)
            FlushWrite();

        Assert(toWrite <= _bufferCapacity - _sizeO);

        memcpy(_bufferO.data() + _sizeO, rawData.data(), toWrite);
        _sizeO += toWrite;

        return toWrite;
    }
}
//----------------------------------------------------------------------------
void FSocketBuffered::FlushRead() {
    FlushRead(Milliseconds(0));
}
//----------------------------------------------------------------------------
void FSocketBuffered::FlushRead(const Milliseconds& timeout) {
    if (_offsetI < _sizeI) {
        Assert(_bufferI);

        const size_t toRead = _sizeI - _offsetI;

        memmove(_bufferI.data(), _bufferI.data() + _offsetI, toRead);

        _sizeI = toRead;
        _offsetI = 0;
    }
    else if (not _bufferI) {
        _bufferI = NewArray<u8>(_bufferCapacity);
    }

    _offsetI += _socket.Read(_bufferI.CutStartingAt(_offsetI), timeout);
}
//----------------------------------------------------------------------------
void FSocketBuffered::FlushWrite() {
    if (_sizeO == 0)
        return;

    Assert(_bufferO);

    _socket.Write(_bufferO.CutBefore(_sizeO));
    _sizeO = 0;
}
//----------------------------------------------------------------------------
bool FSocketBuffered::Accept(FSocketBuffered& buffered, FListener& listener, const Milliseconds& timeout) {
    Assert(false == buffered.IsConnected());

    return listener.Accept(buffered._socket, timeout);
}
//----------------------------------------------------------------------------
bool FSocketBuffered::MakeConnection(FSocketBuffered& buffered, const FAddress& remoteHostnameOrIP) {
    Assert(false == buffered.IsConnected());

    return FSocket::MakeConnection(buffered._socket, remoteHostnameOrIP);
}
//----------------------------------------------------------------------------
size_t FSocketBuffered::ReadFromBuffer_(const TMemoryView<u8>& rawData) {
    const size_t read = Min(rawData.SizeInBytes(), _sizeI - _offsetI);

    _bufferI.SubRange(_offsetI, read).CopyTo(rawData);
    _offsetI += read;

    return read;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
