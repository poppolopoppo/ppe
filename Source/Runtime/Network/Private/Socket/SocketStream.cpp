#include "stdafx.h"

#include "SocketStream.h"

#include "SocketBuffered.h"

namespace PPE {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSocketStreamWriter::FSocketStreamWriter(FSocketBuffered& socket)
:   _socket(socket)
,   _tellO(0) {
    Assert(_socket.IsConnected());
}
//----------------------------------------------------------------------------
FSocketStreamWriter::~FSocketStreamWriter() {}
//----------------------------------------------------------------------------
std::streamoff FSocketStreamWriter::TellO() const {
    return _tellO;
}
//----------------------------------------------------------------------------
std::streamoff FSocketStreamWriter::SeekO(std::streamoff , ESeekOrigin /* = ESeekOrigin::Begin */) {
    AssertNotReached();
}
//----------------------------------------------------------------------------
bool FSocketStreamWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    const size_t count = checked_cast<size_t>(sizeInBytes);
    return (FSocketStreamWriter::WriteSome(storage, 1, count) == count);
}
//----------------------------------------------------------------------------
size_t FSocketStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(_socket.IsConnected());
    Assert(eltsize > 0);

    if (0 == count)
        return 0;

    const TMemoryView<const u8> rawData(static_cast<const u8*>(storage), eltsize*count);
    const size_t written = _socket.Write(rawData);

    _tellO += written;

    Assert(_socket.IsConnected());
    return (written/eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace PPE
