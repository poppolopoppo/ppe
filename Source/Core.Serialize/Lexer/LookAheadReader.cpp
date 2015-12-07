#include "stdafx.h"

#include "LookAheadReader.h"

#include "Core/IO/VFS/VirtualFileSystemStream.h"

#include <locale>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LookAheadReader::LookAheadReader(const StringSlice& input, const wchar_t *sourceFileName)
:   _sourceFileName(sourceFileName)
,   _sourceLine(0)
,   _sourceColumn(0)
,   _buffer(input)
,   _bufferOffset(0) {
    Assert(sourceFileName);
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
    if (Eof())
        return '\0';

    const char value = _buffer[_bufferOffset];
    ++_bufferOffset;

    if ('\n' == value) {
        ++_sourceLine;
        _sourceColumn = 0;
    }
    else {
        ++_sourceColumn;
    }

    return value;
}
//----------------------------------------------------------------------------
char LookAheadReader::Peek(size_t n) const {
    const size_t offset = _bufferOffset + n;

    return (offset >= _buffer.SizeInBytes())
        ? '\0'
        : _buffer[offset];
}
//----------------------------------------------------------------------------
void LookAheadReader::EatWhiteSpaces() {
    while (IsSpace(Peek(0)))
        Read();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
