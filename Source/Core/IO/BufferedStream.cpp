#include "stdafx.h"

#include "BufferedStream.h"

namespace Core {
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
    Assert(_nonBuffered->ToBufferedI() == nullptr);
}
//----------------------------------------------------------------------------
FBufferedStreamReader::~FBufferedStreamReader() {
    if (_buffer) {
        Assert(_nonBuffered);
        FBufferedStreamAllocator::deallocate(_buffer, _bufferSize);
        ONLY_IF_ASSERT(_buffer = nullptr);
    }
}
//----------------------------------------------------------------------------
void FBufferedStreamReader::SetStream(IStreamReader* nonBuffered) {
    Assert(nonBuffered->ToBufferedI() == nullptr);
    _nonBuffered = nonBuffered;
    _origin = 0;
    _offset = 0;
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::Eof() const {
    Assert(_nonBuffered);
    Assert(_origin + _capacity == _nonBuffered->TellI());

    return (_offset == _capacity && _nonBuffered->Eof());
}
//----------------------------------------------------------------------------
std::streamoff FBufferedStreamReader::TellI() const {
    Assert(_nonBuffered);
    Assert(_origin >= 0);
    Assert(_offset <= _capacity);
    Assert(_origin + _capacity == _nonBuffered->TellI());

    return (_origin + _offset);
}
//----------------------------------------------------------------------------
std::streamoff FBufferedStreamReader::SeekI(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(_nonBuffered);
    Assert(_origin >= 0);
    Assert(_origin + _capacity == _nonBuffered->TellI());

    std::streamoff newOrigin;
    switch (origin)
    {
    case Core::ESeekOrigin::Begin:
        newOrigin = offset;
        break;
    case Core::ESeekOrigin::Relative:
        newOrigin = (_origin + _offset) + offset;
        break;
    case Core::ESeekOrigin::End:
        newOrigin = std::streamoff(-1);
        break;
    default:
        AssertNotImplemented();
        newOrigin = std::streamoff(-1);
        break;
    }

    if (newOrigin >= _origin && newOrigin < _origin + _capacity) {
        // Keep current buffer

        _offset = checked_cast<u32>(newOrigin - _origin);
        Assert(TellI() == newOrigin);
    }
    else {
        // Reset overrun buffer

        _origin = _nonBuffered->SeekI(offset, origin);
        _offset = _capacity = 0;
    }

    return (_offset + _offset);
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::Read(void* storage, std::streamsize sizeInBytes) {
    Assert(_nonBuffered);
    Assert(_nonBuffered->TellI() == _origin + _capacity);

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

                ::memcpy(pdst, _buffer + _offset, toRead);

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
            ::memcpy(pdst, _buffer + _offset, toCopy);

            pdst += toCopy;
            sizeInBytes -= toCopy;
        }

        _origin = (_origin + _capacity + sizeInBytes);
        _offset = _capacity = 0;
        if (_nonBuffered->Read(pdst, sizeInBytes)) {
            Assert(_nonBuffered->TellI() == TellI());
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
        if (not FBufferedStreamReader::Read(pdst, eltsize))
            break;
    }

    return read;
}
//----------------------------------------------------------------------------
bool FBufferedStreamReader::Peek(char& ch) {
    Assert(_nonBuffered);
    Assert(_origin + _capacity == _nonBuffered->TellI());

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
    Assert(_origin + _capacity == _nonBuffered->TellI());

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
    Assert(_nonBuffered->TellI() == _origin + _capacity);

    if (_offset && _offset < _capacity) {
        // keep unread relative difference

        const std::streamoff streamoff = _nonBuffered->SeekI(_origin + _offset);
        Assert(std::streamoff(-1) != streamoff);
        Assert(_origin + _offset == streamoff);

        _origin = streamoff;
    }

    if (nullptr == _buffer) {
        // Allocate read buffer lazily
        Assert(_bufferSize > 0);
        _buffer = FBufferedStreamAllocator::allocate(_bufferSize);
        AssertRelease(_buffer);
    }

    const size_t count = _nonBuffered->ReadSome(_buffer, 1, _bufferSize);

    _offset = 0;
    _capacity = checked_cast<u32>(count);

    Assert(_nonBuffered->TellI() == _origin + _capacity);
    return (0 < count);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBufferedStreamWriter::FBufferedStreamWriter()
:   _nonBuffered(nullptr)
,   _buffer(nullptr)
,   _origin(0)
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
    Assert(_nonBuffered->ToBufferedO() == nullptr);
}
//----------------------------------------------------------------------------
FBufferedStreamWriter::~FBufferedStreamWriter() {
    if (_buffer) {
        Assert(_nonBuffered);
        Flush();
        FBufferedStreamAllocator::deallocate(_buffer, _bufferSize);
        ONLY_IF_ASSERT(_buffer = nullptr);
    }
}
//----------------------------------------------------------------------------
void FBufferedStreamWriter::SetStream(IStreamWriter* nonBuffered) {
    Assert(nonBuffered->ToBufferedO() == nullptr);
    Flush();
    Assert(0 == _offset);
    _nonBuffered = nonBuffered;
    _origin = _nonBuffered->TellO();
}
//----------------------------------------------------------------------------
std::streamoff FBufferedStreamWriter::TellO() const {
    Assert(_nonBuffered);
    Assert(_origin >= 0);
    Assert(_offset <= _bufferSize);
    Assert(_nonBuffered->TellO() == _origin);

    return (_origin + _offset);
}
//----------------------------------------------------------------------------
std::streamoff FBufferedStreamWriter::SeekO(std::streamoff offset, ESeekOrigin origin/* = ESeekOrigin::Begin */) {
    Assert(_nonBuffered);
    Assert(_origin >= 0);
    Assert(_nonBuffered->TellO() == _origin);

    std::streamoff newOrigin;
    switch (origin)
    {
    case Core::ESeekOrigin::Begin:
        newOrigin = offset;
        break;
    case Core::ESeekOrigin::Relative:
        newOrigin = (_origin + _offset) + offset;
        break;
    case Core::ESeekOrigin::End:
        newOrigin = std::streamoff(-1);
        break;
    default:
        AssertNotImplemented();
        newOrigin = std::streamoff(-1);
        break;
    }

    if (newOrigin >= _origin && newOrigin < _origin + checked_cast<std::streamoff>(_offset)) {
        // Keep current buffer

        _offset = checked_cast<u32>(newOrigin - _origin);
        Assert(TellO() == newOrigin);
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
    Assert(_nonBuffered->TellO() == _origin);

    if (checked_cast<size_t>(sizeInBytes) <= _bufferSize) {
        // Buffered write

        if (nullptr == _buffer) {
            Assert(_bufferSize);
            _buffer = FBufferedStreamAllocator::allocate(_bufferSize);
            AssertRelease(_buffer);
        }

        const u8* psrc = (const u8*)storage;
        const u8* pend = psrc + checked_cast<ptrdiff_t>(sizeInBytes);
        while (psrc != pend) {
            Assert(_offset <= _bufferSize);

            if (_offset != _bufferSize) {
                const size_t toWrite = Min(checked_cast<size_t>(pend - psrc), _bufferSize - _offset);
                Assert(toWrite > 0);

                ::memcpy(_buffer + _offset, psrc, toWrite);

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
        Assert(TellO() == _nonBuffered->TellO());
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
} //!namespace Core