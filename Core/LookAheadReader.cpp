#include "stdafx.h"

#include "LookAheadReader.h"

#include "VirtualFileSystemStream.h"

#include <locale>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LookAheadReader::LookAheadReader(const StringSlice& input, const char *sourceFileName)
:   _stream(nullptr)
,   _sourceFileName(sourceFileName)
,   _sourceLine(0)
,   _sourceColumn(0)
,   _bufferOffset(0)
,   _bufferSize(input.size())
,   _eof(true) {
    static_assert(IS_POW2(BufferCapacity), "BufferCapacity must be a power of 2");

    Assert(sourceFileName);
    Assert(input.size() <= BufferCapacity);

    memcpy(_buffer, input.Pointer(), input.SizeInBytes());
}
//----------------------------------------------------------------------------
LookAheadReader::LookAheadReader(IVirtualFileSystemIStream *stream, const char *sourceFileName)
:   _stream(stream)
,   _sourceFileName(sourceFileName)
,   _sourceLine(0)
,   _sourceColumn(0)
,   _bufferOffset(0)
,   _bufferSize(0)
,   _eof(false) {
    static_assert(IS_POW2(BufferCapacity), "BufferCapacity must be a power of 2");

    Assert(stream);
    Assert(sourceFileName);

    RefillBuffer_(0, BufferCapacity);
}
//----------------------------------------------------------------------------
LookAheadReader::~LookAheadReader() {}
//----------------------------------------------------------------------------
void LookAheadReader::SeekForward(size_t offset) {
    while (offset--)
        Read();
}
//----------------------------------------------------------------------------
char LookAheadReader::Read() {
    if (0 == _bufferSize)
        return '\0';

    Assert(_bufferOffset < BufferCapacity);

    const char value = _buffer[_bufferOffset];
    _bufferOffset = ++_bufferOffset & BufferMask;
    --_bufferSize;

    if ('\n' == value) {
        ++_sourceLine;
        _sourceColumn = 0;
    }
    else {
        ++_sourceColumn;
    }

    if (!_eof) {
        if (0 == _bufferOffset)
            RefillBuffer_(HalfBufferCapacity, HalfBufferCapacity);
        else if (HalfBufferCapacity == _bufferOffset)
            RefillBuffer_(0, HalfBufferCapacity);
    }

    return value;
}
//----------------------------------------------------------------------------
char LookAheadReader::Peek(size_t n) const {
    Assert(n < HalfBufferCapacity);

    if (n >= _bufferSize) {
        Assert(_eof);
        return '\0';
    }

    const char value = _buffer[(_bufferOffset + n) & BufferMask];

    return value;
}
//----------------------------------------------------------------------------
void LookAheadReader::EatWhiteSpaces() {
    const std::locale& locale = std::locale::classic();
    while (std::isspace(Peek(0), locale))
        Read();
}
//----------------------------------------------------------------------------
void LookAheadReader::RefillBuffer_(size_t index, size_t length) {
    Assert(_stream);
    Assert(!_eof);
    Assert(0 <= index);
    Assert(index + length <= BufferCapacity);

    const size_t readCount = checked_cast<size_t>(_stream->ReadSome(&_buffer[index], length));

    _eof = (readCount < length);
    _bufferSize += readCount;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
