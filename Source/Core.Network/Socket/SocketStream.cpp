#include "stdafx.h"

#include "SocketStream.h"

#include "SocketBuffered.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSocketStreamWriter::FSocketStreamWriter(FSocketBuffered* psocket)
:   _tellO(0)
,   _psocket(psocket) {
    Assert(_psocket);
    Assert(_psocket->IsConnected());
}
//----------------------------------------------------------------------------
FSocketStreamWriter::~FSocketStreamWriter() {}
//----------------------------------------------------------------------------
std::streamoff FSocketStreamWriter::TellO() const {
    return _tellO;
}
//----------------------------------------------------------------------------
bool FSocketStreamWriter::SeekO(std::streamoff offset, ESeekOrigin policy/* = ESeekOrigin::Begin */) {
    AssertNotReached();
    return false;
}
//----------------------------------------------------------------------------
bool FSocketStreamWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    const size_t count = checked_cast<size_t>(sizeInBytes);
    return (FSocketStreamWriter::WriteSome(storage, 1, count) == count);
}
//----------------------------------------------------------------------------
size_t FSocketStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(_psocket);
    Assert(_psocket->IsConnected());
    Assert(eltsize > 0);

    if (0 == count)
        return 0;

    const TMemoryView<const u8> rawData(static_cast<const u8*>(storage), eltsize*count);
    const size_t written = _psocket->Write(rawData);

    _tellO += written;

    Assert(_psocket->IsConnected());
    return (written/eltsize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
