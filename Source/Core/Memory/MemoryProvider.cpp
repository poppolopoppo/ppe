#include "stdafx.h"

#include "MemoryProvider.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MemoryViewReader::SeekI(std::streamoff offset, SeekOrigin origin /* = SeekOrigin::Begin */) {
    switch (origin)
    {
    case SeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_rawData.SizeInBytes()))
            return false;
        _offsetI = checked_cast<size_t>(offset);
        break;
    case SeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetI) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetI) + offset > checked_cast<std::streamoff>(_rawData.SizeInBytes()) )
            return false;
        _offsetI = checked_cast<size_t>(_offsetI + offset);
        break;
    case SeekOrigin::End:
        if (checked_cast<std::streamoff>(_rawData.SizeInBytes()) + offset < 0 ||
            checked_cast<std::streamoff>(_rawData.SizeInBytes()) + offset > checked_cast<std::streamoff>(_rawData.SizeInBytes()) )
            return false;
        _offsetI = checked_cast<size_t>(_rawData.SizeInBytes() + offset);
        break;
    default:
        AssertNotImplemented();
        return false;
    }
    Assert(_offsetI <= _rawData.SizeInBytes());
    return true;
}
//----------------------------------------------------------------------------
bool MemoryViewReader::Read(void* storage, std::streamsize sizeInBytes) {
    const size_t sizeT = checked_cast<size_t>(sizeInBytes);
    if (_offsetI + sizeT > _rawData.SizeInBytes())
        return false;

    Assert(storage);
    memcpy(storage, _rawData.Pointer() + _offsetI, sizeT);

    _offsetI += sizeT;
    return true;
}
//----------------------------------------------------------------------------
std::streamsize MemoryViewReader::ReadSome(void* storage, size_t eltsize, std::streamsize count) {
    Assert(_rawData.SizeInBytes() >= _offsetI);
    Assert(eltsize > 0);
    const std::streamsize remaining = checked_cast<std::streamsize>(_rawData.SizeInBytes() - _offsetI);
    const std::streamsize wantedsize = eltsize*count;
    const std::streamsize realsize = remaining < wantedsize ? remaining : wantedsize;
    return (MemoryViewReader::Read(storage, realsize) ) ? realsize : 0;
}
//----------------------------------------------------------------------------
bool MemoryViewReader::Peek(char& ch) {
    Assert(_offsetI <= _rawData.SizeInBytes());
    if (_offsetI == _rawData.SizeInBytes())
        return false;

    ch = checked_cast<char>(_rawData[_offsetI]);
    return true;
}
//----------------------------------------------------------------------------
bool MemoryViewReader::Peek(wchar_t& wch) {
    Assert(_offsetI <= _rawData.SizeInBytes());
    if (_offsetI + sizeof(wchar_t) > _rawData.SizeInBytes())
        return false;

    wch = *reinterpret_cast<const wchar_t*>(&_rawData[_offsetI]);
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::streamoff MemoryViewWriter::TellO() const {
    Assert(_offsetO <= _size);
    Assert(_size <= _rawData.SizeInBytes());
    return checked_cast<std::streamsize>(_offsetO);
}
//----------------------------------------------------------------------------
bool MemoryViewWriter::SeekO(std::streamoff offset, SeekOrigin policy /* = SeekOrigin::Begin */) {
    switch (policy)
    {
    case SeekOrigin::Begin:
        if (offset > checked_cast<std::streamoff>(_size))
            return false;
        _offsetO = checked_cast<size_t>(offset);
        break;
    case SeekOrigin::Relative:
        if (checked_cast<std::streamoff>(_offsetO) + offset < 0 ||
            checked_cast<std::streamoff>(_offsetO) + offset > checked_cast<std::streamoff>(_size) )
            return false;
        _offsetO = checked_cast<size_t>(_offsetO + offset);
        break;
    case SeekOrigin::End:
        if (checked_cast<std::streamoff>(_size) + offset < 0 ||
            checked_cast<std::streamoff>(_size) + offset > checked_cast<std::streamoff>(_size) )
            return false;
        _offsetO = checked_cast<size_t>(_size + offset);
        break;
    default:
        AssertNotImplemented();
        return false;
    }
    Assert(_offsetO <= _size);
    return true;
}
//----------------------------------------------------------------------------
bool MemoryViewWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    const size_t sizeT = checked_cast<size_t>(sizeInBytes);

    if (_offsetO + sizeT > _rawData.SizeInBytes()) {
        AssertNotReached();
        return false;
    }

    memcpy(_rawData.Pointer() + _offsetO, storage, sizeT);

    _offsetO += sizeT;
    _size = std::max(_offsetO, _size);
    return true;
}
//----------------------------------------------------------------------------
bool MemoryViewWriter::WriteSome(const void* storage, size_t eltsize, std::streamsize count) {
    return MemoryViewWriter::Write(storage, eltsize * count);
}
//----------------------------------------------------------------------------
bool MemoryViewWriter::WriteAligned(const void* storage, std::streamsize sizeInBytes, size_t boundary) {
    return (WriteAlignmentPadding(boundary) && Write(storage, sizeInBytes) );
}
//----------------------------------------------------------------------------
bool MemoryViewWriter::WriteAlignmentPadding(size_t boundary, u8 padvalue /* = 0 */) {
    Assert(boundary > 0 && IS_POW2(boundary));
    const size_t offset = (_offsetO + boundary - 1) & ~(boundary - 1);
    if (offset > _rawData.SizeInBytes()) {
        AssertNotReached();
        return false;
    }

    if (_offsetO != offset) {
        Assert(_offsetO < offset);
        _size = std::max(_size, offset);
        memset(_rawData.Pointer() + _offsetO, padvalue, offset - _offsetO);
        _offsetO = offset;
    }

    Assert(IS_ALIGNED(boundary, _rawData.Pointer() + _offsetO));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core