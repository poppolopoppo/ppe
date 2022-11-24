// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/BufferedStream.h"

#include "HAL/PlatformMemory.h"
#include "Misc/Function.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBufferedStreamReader::FBufferedStreamReader()
    : _nonBuffered(nullptr)
    , _buffer(nullptr)
    , _origin(0)
    , _offset(0)
    , _capacity(0)
    , _bufferSize(GBufferedStreamDefaultBufferSize)
{}
//----------------------------------------------------------------------------
FBufferedStreamReader::FBufferedStreamReader(IStreamReader* nonBuffered, size_t bufferSize/* = GBufferedStreamDefaultBufferSize */)
    : _nonBuffered(nonBuffered)
    , _buffer(nullptr)
    , _origin(_nonBuffered->TellI())
    , _offset(0)
    , _capacity(0)
    , _bufferSize(bufferSize) {
    Assert(_bufferSize);
    Assert(_nonBuffered);
    Assert_NoAssume(_nonBuffered->ToBufferedI() == nullptr);
}
//----------------------------------------------------------------------------
FBufferedStreamReader::~FBufferedStreamReader() {
    if (_buffer) {
        Assert(_nonBuffered);
        FBufferedStreamAllocator::Deallocate(FAllocatorBlock{ _buffer, _bufferSize });
        ONLY_IF_ASSERT(_buffer = nullptr);
    }
}
//----------------------------------------------------------------------------
void FBufferedStreamReader::ForceAllocateInnerBuffer() {
    Assert(nullptr == _buffer);
    Assert(_bufferSize);

    _bufferSize = FBufferedStreamAllocator::SnapSize(_bufferSize);
    _buffer = static_cast<u8*>(FBufferedStreamAllocator::Allocate(_bufferSize).Data);

    AssertRelease(_buffer);
}
//----------------------------------------------------------------------------
void FBufferedStreamReader::SetStream(IStreamReader* nonBuffered) {
    Assert(nonBuffered->ToBufferedI() == nullptr);
    _nonBuffered = nonBuffered;
    _origin = 0;
    _offset = 0;
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::ReadAt_SkipBuffer(const FRawMemory& storage, std::streamoff absolute) {
    Assert(storage.size());

    const std::streamoff org = _nonBuffered->TellI();
    const bool success = _nonBuffered->ReadAt(storage, absolute);

    _nonBuffered->SeekI(org, ESeekOrigin::Begin);

    return success;
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::Eof() const NOEXCEPT {
    Assert(_nonBuffered);
    Assert_NoAssume(_origin + _capacity == _nonBuffered->TellI());

    return (_offset == _capacity && _nonBuffered->Eof());
}
//----------------------------------------------------------------------------
std::streamoff FBufferedStreamReader::TellI() const NOEXCEPT {
    Assert(_nonBuffered);
    Assert(_origin >= 0);
    Assert(_offset <= _capacity);
    Assert_NoAssume(_origin + _capacity == _nonBuffered->TellI());

    return (_origin + _offset);
}
//----------------------------------------------------------------------------
std::streamoff FBufferedStreamReader::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(_nonBuffered);
    Assert(_origin >= 0);
    Assert_NoAssume(_origin + _capacity == _nonBuffered->TellI());

    std::streamoff newOrigin;
    switch (origin) {
    case PPE::ESeekOrigin::Begin:
        newOrigin = offset;
        break;
    case PPE::ESeekOrigin::Relative:
        newOrigin = (_origin + _offset) + offset;
        break;
    case PPE::ESeekOrigin::End:
        newOrigin = std::streamoff(-1);
        break;
    default:
        AssertNotImplemented();
    }

    if (newOrigin >= _origin && newOrigin < _origin + _capacity) {
        // Keep current buffer

        _offset = checked_cast<u32>(newOrigin - _origin);
        Assert_NoAssume(TellI() == newOrigin);
    }
    else {
        // Reset overrun buffer

        _origin = _nonBuffered->SeekI(offset, origin);
        _offset = _capacity = 0;
    }

    return (_origin + _offset);
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::Read(void* storage, std::streamsize sizeInBytes) {
    Assert(_nonBuffered);
    Assert_NoAssume(_nonBuffered->TellI() == _origin + _capacity);

    u8* pdst = (u8*)storage;

    if (checked_cast<size_t>(sizeInBytes) <= _bufferSize) {
        // Buffered read

        u8* pend = pdst + checked_cast<ptrdiff_t>(sizeInBytes);
        while (pdst != pend) {
            Assert(_offset <= _capacity);

            if (_offset != _capacity) {
                Assert(_buffer);
                Assert(_capacity);

                const u32 toRead = Min(checked_cast<u32>(pend - pdst), _capacity - _offset);
                Assert(toRead > 0);

                FPlatformMemory::Memcpy(pdst, _buffer + _offset, toRead);

                pdst += toRead;
                _offset += toRead;
            }
            else if (not RefillBuffer_()) {
                return false;
            }
        }
        return true;
    }
    else {
        // Non buffered read

        Assert(_offset <= _capacity);
        if (_offset != _capacity) {
            Assert(_buffer);

            const size_t toCopy = (_capacity - _offset);
            FPlatformMemory::MemcpyLarge(pdst, _buffer + _offset, toCopy);

            pdst += toCopy;
            sizeInBytes -= toCopy;
        }

        _origin = (_origin + _capacity + sizeInBytes);
        _offset = _capacity = 0;
        if (_nonBuffered->Read(pdst, sizeInBytes)) {
            Assert_NoAssume(_nonBuffered->TellI() == TellI());
            return true;
        }
        else {
            _origin = _nonBuffered->TellI();
            return false;
        }
    }
}
//----------------------------------------------------------------------------
size_t FBufferedStreamReader::ReadSome(void* storage, size_t eltsize, size_t count) {
    Assert(_nonBuffered);
    Assert(eltsize);

    size_t read = 0;
    u8* pdst = (u8*)storage;
    for (; read < count; read++, pdst += eltsize) {
        if (Likely(_offset + eltsize <= _capacity)) {
            FPlatformMemory::Memcpy(pdst, _buffer + _offset, eltsize);
            _offset = checked_cast<u32>(_offset + eltsize);
        }
        else {
            if (not FBufferedStreamReader::Read(pdst, eltsize))
                break;
        }
    }

    return read;
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::Peek(char& ch) {
    Assert(_nonBuffered);
    Assert_NoAssume(_origin + _capacity == _nonBuffered->TellI());

    if (_offset + sizeof(char) > _capacity && not RefillBuffer_()) {
        ch = char(EOF);
        return false;
    }
    else {
        Assert(_offset + sizeof(char) <= _capacity);
        ch = *(const char*)(_buffer + _offset);
        return true;
    }
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::Peek(wchar_t& wch) {
    Assert(_nonBuffered);
    Assert_NoAssume(_origin + _capacity == _nonBuffered->TellI());

    if (_offset + sizeof(wchar_t) > _capacity && not RefillBuffer_()) {
        wch = wchar_t(EOF);
        return false;
    }
    else {
        Assert(_offset + sizeof(wchar_t) <= _capacity);
        wch = *(const wchar_t*)(_buffer + _offset);
        return true;
    }
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::RefillBuffer_() {
    Assert(_nonBuffered);
    Assert_NoAssume(_nonBuffered->TellI() == _origin + _capacity);

    if (_capacity & _offset) {
        // keep unread relative difference

        const std::streamoff streamoff = _nonBuffered->SeekI(_origin + _offset);
        Assert(std::streamoff(-1) != streamoff);
        Assert(_origin + _offset == streamoff);

        _origin = streamoff;
    }

    _offset = 0;

    // skip call to ReadSome() if the buffer is empty and ate of internal stream
    if (not _nonBuffered->Eof()) {
        if (nullptr == _buffer) // allocate read buffer lazily IFN
            ForceAllocateInnerBuffer();

        _capacity = checked_cast<u32>(_nonBuffered->ReadSome(_buffer, 1, _bufferSize));
    }
    else {
        _capacity = 0;
    }

    Assert_NoAssume(_nonBuffered->TellI() == _origin + _capacity);
    return (_capacity != 0);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBufferedStreamWriter::FBufferedStreamWriter()
:   _nonBuffered(nullptr)
,   _buffer(nullptr)
,   _origin(0)
,   _offset(0)
,   _bufferSize(GBufferedStreamDefaultBufferSize)
{}
//----------------------------------------------------------------------------
FBufferedStreamWriter::FBufferedStreamWriter(IStreamWriter* nonBuffered, size_t bufferSize/* = GBufferedStreamDefaultBufferSize */)
:   _nonBuffered(nonBuffered)
,   _buffer(nullptr)
,   _origin(_nonBuffered->TellO())
,   _offset(0)
,   _bufferSize(bufferSize) {
    Assert(_bufferSize);
    Assert(_nonBuffered);
    Assert_NoAssume(_nonBuffered->ToBufferedO() == nullptr);
}
//----------------------------------------------------------------------------
FBufferedStreamWriter::~FBufferedStreamWriter() {
    if (_nonBuffered) {
        Flush();
        ONLY_IF_ASSERT(_nonBuffered = nullptr);
    }
    if (_buffer) {
        FBufferedStreamAllocator::Deallocate(FAllocatorBlock{ _buffer, _bufferSize });
        ONLY_IF_ASSERT(_buffer = nullptr);
    }
}
//----------------------------------------------------------------------------
void FBufferedStreamWriter::ForceAllocateInnerBuffer() {
    Assert(nullptr == _buffer);
    Assert(_bufferSize);

    _bufferSize = FBufferedStreamAllocator::SnapSize(_bufferSize);
    _buffer = static_cast<u8*>(FBufferedStreamAllocator::Allocate(_bufferSize).Data);

    AssertRelease(_buffer);
}
//----------------------------------------------------------------------------
void FBufferedStreamWriter::SetStream(IStreamWriter* nonBuffered) {
    Assert_NoAssume(nonBuffered->ToBufferedO() == nullptr);
    Flush();
    Assert(0 == _offset);
    _nonBuffered = nonBuffered;
    _origin = _nonBuffered->TellO();
}
//----------------------------------------------------------------------------
void FBufferedStreamWriter::ResetStream() {
    Assert(_nonBuffered);
    Flush();
    Assert(0 == _offset);
    _nonBuffered = nullptr;
    _origin = 0;
}
//----------------------------------------------------------------------------
std::streamoff FBufferedStreamWriter::TellO() const NOEXCEPT {
    Assert(_nonBuffered);
    Assert(_origin >= 0);
    Assert(_offset <= _bufferSize);
    Assert_NoAssume(_nonBuffered->TellO() == _origin);

    return (_origin + _offset);
}
//----------------------------------------------------------------------------
std::streamoff FBufferedStreamWriter::SeekO(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(_nonBuffered);
    Assert(_origin >= 0);
    Assert_NoAssume(_nonBuffered->TellO() == _origin);

    std::streamoff newOrigin;
    switch (origin)
    {
    case PPE::ESeekOrigin::Begin:
        newOrigin = offset;
        break;
    case PPE::ESeekOrigin::Relative:
        newOrigin = (_origin + _offset) + offset;
        break;
    case PPE::ESeekOrigin::End:
        newOrigin = std::streamoff(-1);
        break;
    default:
        AssertNotImplemented();
    }

    if (newOrigin >= _origin && newOrigin < _origin + checked_cast<std::streamoff>(_offset)) {
        // Keep current buffer

        _offset = checked_cast<u32>(newOrigin - _origin);
        Assert_NoAssume(TellO() == newOrigin);
    }
    else {
        // Reset overrun buffer

        CommitBuffer_();
        _origin = _nonBuffered->SeekO(offset, origin);
    }

    return (_origin + _offset);
}
//----------------------------------------------------------------------------
bool FBufferedStreamWriter::Write(const void* storage, std::streamsize sizeInBytes) {
    Assert(_nonBuffered);
    Assert_NoAssume(_nonBuffered->TellO() == _origin);

    if (checked_cast<size_t>(sizeInBytes) <= _bufferSize) {
        // Buffered write

        if (nullptr == _buffer) // allocate write buffer lazily IFN
            ForceAllocateInnerBuffer();

        const u8* psrc = (const u8*)storage;
        const u8* pend = psrc + checked_cast<ptrdiff_t>(sizeInBytes);
        while (psrc != pend) {
            Assert(_offset <= _bufferSize);

            if (_offset != _bufferSize) {
                const size_t toWrite = Min(checked_cast<size_t>(pend - psrc), _bufferSize - _offset);
                Assert(toWrite > 0);

                FPlatformMemory::Memcpy(_buffer + _offset, psrc, toWrite);

                psrc += toWrite;
                _offset += toWrite;
            }
            else if (not CommitBuffer_()) {
                return false;
            }
        }
        return true;
    }
    else {
        // Commit buffer and native write

        const bool succeed = (CommitBuffer_() && _nonBuffered->Write(storage, sizeInBytes));
        _origin = _nonBuffered->TellO();

        return succeed;
    }
}
//----------------------------------------------------------------------------
size_t FBufferedStreamWriter::WriteSome(const void* storage, size_t eltsize, size_t count) {
    Assert(_nonBuffered);
    Assert(eltsize);

    size_t written = 0;
    const u8* psrc = (const u8*)storage;
    for (; written < count; written++, psrc += eltsize) {
        if (not FBufferedStreamWriter::Write(psrc, eltsize))
            break;
    }

    return written;
}
//----------------------------------------------------------------------------
size_t FBufferedStreamWriter::StreamCopy(const read_f& read, size_t blockSz) {
    blockSz = Min(_bufferSize - _offset, blockSz);
    Assert(blockSz);

    blockSz = read(FRawMemory(_buffer + _offset, blockSz));

    _offset += blockSz;
    if (_offset == _bufferSize)
        Verify(CommitBuffer_());

    return blockSz;
}
//----------------------------------------------------------------------------
void FBufferedStreamWriter::Flush() {
    VerifyRelease(CommitBuffer_());
}
//----------------------------------------------------------------------------
bool FBufferedStreamWriter::CommitBuffer_() {
    Assert(_nonBuffered);
    Assert(_nonBuffered->TellO() == _origin);

    if (0 == _offset)
        return true;

    Assert(_buffer);

    const std::streamsize toWrite = checked_cast<std::streamsize>(_offset);

    if (_nonBuffered->Write(_buffer, toWrite)) {
        _origin += toWrite;
        _offset = 0;
        Assert_NoAssume(TellO() == _nonBuffered->TellO());
        return true;
    }
    else {
        _nonBuffered->SeekO(_origin, ESeekOrigin::Begin);
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
