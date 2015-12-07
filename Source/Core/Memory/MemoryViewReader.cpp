#include "stdafx.h"

#include "MemoryViewReader.h"

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
    return (MemoryViewReader::Read(storage, eltsize * count) ) ? realsize : 0;
}
//----------------------------------------------------------------------------
char MemoryViewReader::PeekChar() {
    Assert(_offsetI <= _rawData.SizeInBytes());
    if (_offsetI == _rawData.SizeInBytes())
        return '\0';

    return checked_cast<char>(_rawData[_offsetI]);
}
//----------------------------------------------------------------------------
wchar_t MemoryViewReader::PeekCharW() {
    Assert(_offsetI <= _rawData.SizeInBytes());
    if (_offsetI + sizeof(wchar_t) > _rawData.SizeInBytes())
        return L'\0';

    return *reinterpret_cast<const wchar_t*>(&_rawData[_offsetI]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core