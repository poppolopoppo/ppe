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
bool FSocketBuffered::IsReadable(const FMilliseconds& timeout) const {
    return (not _bufferI.empty() || _socket.IsReadable(timeout));
}
//----------------------------------------------------------------------------
size_t FSocketBuffered::Read(const TMemoryView<u8>& rawData) {
    return Read(rawData, FMilliseconds(0)/* no timeout, no wait */);
}
//----------------------------------------------------------------------------
size_t FSocketBuffered::Read(const TMemoryView<u8>& rawData, const FMilliseconds& timeout) {
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

        if (nullptr == _bufferO)
            _bufferO = NewArray<u8>(_bufferCapacity);

        Assert(toWrite <= _bufferCapacity - _sizeO);

        memcpy(_bufferO.data() + _sizeO, rawData.data(), toWrite);
        _sizeO += toWrite;

        return toWrite;
    }
}
//----------------------------------------------------------------------------
void FSocketBuffered::EatWhiteSpaces() {
    char ch;
    while (Peek(ch) && IsSpace(ch)) {
        ++_offsetI;
        if (ch == '\n')
            break;
    }
}
//----------------------------------------------------------------------------
bool FSocketBuffered::ReadUntil(std::ostream* poss, char delim) {
    constexpr size_t maxLength = 64 * 1024;

    char ch;
    for(size_t len = 0;
        Peek(ch) && ch != delim && ch != '\n';
        ++len ) {
        if (len == maxLength)
            return false;

        if (not Get(ch))
            AssertNotReached();

        poss->put(ch);
    }

    if (not Peek(ch))
        return false;

    return true;
}
//----------------------------------------------------------------------------
void FSocketBuffered::FlushRead() {
    FlushRead(FMilliseconds(0));
}
//----------------------------------------------------------------------------
void FSocketBuffered::FlushRead(const FMilliseconds& timeout) {
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

    _sizeI += _socket.Read(_bufferI.CutStartingAt(_offsetI), timeout);
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
bool FSocketBuffered::Accept(FSocketBuffered& buffered, FListener& listener, const FMilliseconds& timeout) {
    Assert(false == buffered.IsConnected());

    buffered._offsetI = buffered._sizeI = buffered._sizeO = 0;
    return listener.Accept(buffered._socket, timeout);
}
//----------------------------------------------------------------------------
bool FSocketBuffered::MakeConnection(FSocketBuffered& buffered, const FAddress& remoteHostnameOrIP) {
    Assert(false == buffered.IsConnected());

    buffered._offsetI = buffered._sizeI = buffered._sizeO = 0;
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
