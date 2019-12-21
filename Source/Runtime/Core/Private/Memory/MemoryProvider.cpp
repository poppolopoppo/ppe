#include "stdafx.h"

#include "Memory/MemoryProvider.h"

#include "HAL/PlatformMemory.h"
#include "Misc/Function.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::streamoff FMemoryViewReader::SeekI(std::streamoff offset, ESeekOrigin origin /* = ESeekOrigin::Begin */) {
    switch (origin)
    {
    case ESeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_rawData.SizeInBytes()))
            return std::streamoff(-1);
        _offsetI = checked_cast<size_t>(offset);
        break;
    case ESeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetI) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetI) + offset > checked_cast<std::streamoff>(_rawData.SizeInBytes()) )
            return std::streamoff(-1);
        _offsetI = checked_cast<size_t>(_offsetI + offset);
        break;
    case ESeekOrigin::End:
        if (checked_cast<std::streamoff>(_rawData.SizeInBytes()) + offset < 0 ||
            checked_cast<std::streamoff>(_rawData.SizeInBytes()) + offset > checked_cast<std::streamoff>(_rawData.SizeInBytes()) )
            return std::streamoff(-1);
        _offsetI = checked_cast<size_t>(_rawData.SizeInBytes() + offset);
        break;
    case ESeekOrigin::All:
        AssertNotReached();
    }
    Assert(_offsetI <= _rawData.SizeInBytes());
    return checked_cast<std::streamoff>(_offsetI);
}
//----------------------------------------------------------------------------
bool FMemoryViewReader::Read(void* storage, std::streamsize sizeInBytes) {
    const size_t sizeT = checked_cast<size_t>(sizeInBytes);
    if (_offsetI + sizeT > _rawData.SizeInBytes())
        return false;

    Assert(storage);
    FPlatformMemory::Memcpy(storage, _rawData.Pointer() + _offsetI, sizeT);

    _offsetI += sizeT;
    return true;
}
//----------------------------------------------------------------------------
size_t FMemoryViewReader::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(_rawData.SizeInBytes() >= _offsetI);
    Assert(eltsize > 0);

    const size_t remaining = (_rawData.SizeInBytes() - _offsetI);
    const size_t wantedsize = eltsize*count;
    const size_t realsize = remaining < wantedsize ? remaining : wantedsize;

    return (FMemoryViewReader::Read(storage, realsize) ? (realsize/eltsize) : 0);
}
//----------------------------------------------------------------------------
bool FMemoryViewReader::Peek(char& ch) {
    Assert(_offsetI <= _rawData.SizeInBytes());
    if (_offsetI == _rawData.SizeInBytes())
        return false;

    ch = checked_cast<char>(_rawData[_offsetI]);
    return true;
}
//----------------------------------------------------------------------------
bool FMemoryViewReader::Peek(wchar_t& wch) {
    Assert(_offsetI <= _rawData.SizeInBytes());
    if (_offsetI + sizeof(wchar_t) > _rawData.SizeInBytes())
        return false;

    wch = *reinterpret_cast<const wchar_t*>(&_rawData[_offsetI]);
    return true;
}
//----------------------------------------------------------------------------
bool FMemoryViewReader::ReadAt_SkipBuffer(const FRawMemory& storage, std::streamoff absolute) {
    if (absolute + storage.size() <= _rawData.size()) {
        _rawData.SubRange(checked_cast<size_t>(absolute), storage.size()).CopyTo(storage);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::streamoff FMemoryViewWriter::TellO() const {
    Assert(_offsetO <= _size);
    Assert(_size <= _rawData.SizeInBytes());
    return checked_cast<std::streamsize>(_offsetO);
}
//----------------------------------------------------------------------------
std::streamoff FMemoryViewWriter::SeekO(std::streamoff offset, ESeekOrigin policy /* = ESeekOrigin::Begin */) {
    switch (policy)
    {
    case ESeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_size))
            return std::streamoff(-1);
        _offsetO = checked_cast<size_t>(offset);
        break;
    case ESeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetO) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetO) + offset > checked_cast<std::streamoff>(_size) )
            return std::streamoff(-1);
        _offsetO = checked_cast<size_t>(_offsetO + offset);
        break;
    case ESeekOrigin::End:
        if (checked_cast<std::streamoff>(_size) + offset < 0 ||
            checked_cast<std::streamoff>(_size) + offset > checked_cast<std::streamoff>(_size) )
            return std::streamoff(-1);
        _offsetO = checked_cast<size_t>(_size + offset);
        break;
    case ESeekOrigin::All:
        AssertNotReached();
    }
    Assert(_offsetO <= _size);
    return checked_cast<std::streamoff>(_offsetO);
}
//----------------------------------------------------------------------------
bool FMemoryViewWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    const size_t sizeT = checked_cast<size_t>(sizeInBytes);

    AssertRelease(_offsetO + sizeT <= _rawData.SizeInBytes());

    FPlatformMemory::Memcpy(_rawData.Pointer() + _offsetO, storage, sizeT);

    _offsetO += sizeT;
    _size = Max(_offsetO, _size);
    return true;
}
//----------------------------------------------------------------------------
size_t FMemoryViewWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    return (FMemoryViewWriter::Write(storage, eltsize * count) ? count : 0);
}
//----------------------------------------------------------------------------
bool FMemoryViewWriter::WriteAligned(const void* storage, std::streamsize sizeInBytes, size_t boundary) {
    return (WriteAlignmentPadding(boundary) && Write(storage, sizeInBytes) );
}
//----------------------------------------------------------------------------
bool FMemoryViewWriter::WriteAlignmentPadding(size_t boundary, u8 padvalue /* = 0 */) {
    Assert(boundary > 0 && Meta::IsPow2(boundary));

    const size_t offset = (_offsetO + boundary - 1) & ~(boundary - 1);

    AssertRelease(offset <= _rawData.SizeInBytes());

    if (_offsetO != offset) {
        Assert(_offsetO < offset);
        _size = Max(_size, offset);
        FPlatformMemory::Memset(_rawData.Pointer() + _offsetO, padvalue, offset - _offsetO);
        _offsetO = offset;
    }

    Assert(Meta::IsAligned(boundary, _rawData.Pointer() + _offsetO));
    return true;
}
//----------------------------------------------------------------------------
size_t FMemoryViewWriter::StreamCopy(const read_f& read, size_t blockSz) {
    blockSz = Min(_rawData.SizeInBytes() - _offsetO, blockSz);
    blockSz = read(_rawData.SubRange(_offsetO, blockSz));

    _offsetO += blockSz;
    _size = Max(_offsetO, _size);

    return blockSz;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
