#include "stdafx.h"

#include "LookAheadReader.h"

#include "Core/IO/StreamProvider.h"

#include <locale>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(LookAheadReader) == 2048);
//----------------------------------------------------------------------------
LookAheadReader::LookAheadReader(IStreamReader* input, const wchar_t *sourceFileName)
:   _sourceFileName(sourceFileName)
,   _sourceLine(1)
,   _sourceColumn(1)
,   _input(input)
,   _bufferPos(0)
,   _bufferSize(0) {
    Assert(input);
    Assert(sourceFileName);
}
//----------------------------------------------------------------------------
LookAheadReader::~LookAheadReader() {}
//----------------------------------------------------------------------------
bool LookAheadReader::Eof() const {
    return (_bufferPos == _bufferPos && _input->Eof());
}
//----------------------------------------------------------------------------
size_t LookAheadReader::Tell() const {
    const std::streamsize streamPos = _input->TellI();
    Assert(_bufferPos <= _bufferSize);
    const std::streamsize bufferRemaining(_bufferSize - _bufferPos);
    Assert(streamPos >= bufferRemaining);
    return checked_cast<size_t>(streamPos - bufferRemaining);
}
//----------------------------------------------------------------------------
void LookAheadReader::SeekFwd(size_t offset) {
    while (offset--)
        Read();
}
//----------------------------------------------------------------------------
char LookAheadReader::Peek(size_t n/* = 0 */) const {
    if (_bufferPos + n >= _bufferSize) {
        const_cast<LookAheadReader*>(this)->Flush();

        if ((_bufferPos + n >= _bufferSize))
            return '\0';
    }

    return char(_bufferData[_bufferPos + n]);
}
//----------------------------------------------------------------------------
char LookAheadReader::Read() {
    if (_bufferSize == _bufferPos)
        Flush();

    if (0 == _bufferSize)
        return '\0';

    Assert(_bufferPos < _bufferSize);
    const char value(_bufferData[_bufferPos++]);

    if ('\n' == value) {
        ++_sourceLine;
        _sourceColumn = 1;
    }
    else {
        ++_sourceColumn;
    }

    return value;
}
//----------------------------------------------------------------------------
bool LookAheadReader::ReadUntil(String& dst, char expected) {
    while (char read = Peek()) {
        if (expected == read)
            return true;

        dst += Read();
    }
    return false;
}
//----------------------------------------------------------------------------
bool LookAheadReader::SkipUntil(char expected) {
    while (char read = Peek()) {
        if (expected == read)
            return true;

        Read();
    }
    return false;
}
//----------------------------------------------------------------------------
void LookAheadReader::EatWhiteSpaces() {
    while (IsSpace(Peek(0)))
        Read();
}
//----------------------------------------------------------------------------
void LookAheadReader::Flush() {
    if (0 == _bufferPos && 0 < _bufferSize) {
        return;
    }
    else if (_bufferPos < _bufferSize) {
        const size_t bufferRemaining = (_bufferSize - _bufferPos);
        memmove(&_bufferData[0], &_bufferData[_bufferPos], bufferRemaining);

        _bufferPos = 0;
        _bufferSize = bufferRemaining;
    }
    else {
        _bufferPos = _bufferSize = 0;
    }

    const size_t bufferToFill = (BufferCapacity - _bufferSize);
    if (bufferToFill) {
        const std::streamsize read = _input->ReadSome(&_bufferData[_bufferSize], 1, bufferToFill);
        _bufferSize += checked_cast<size_t>(read);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
