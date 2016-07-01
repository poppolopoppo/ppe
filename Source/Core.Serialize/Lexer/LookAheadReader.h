#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core/IO/StringSlice.h"

namespace Core {
class IVirtualFileSystemIStream;

namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LookAheadReader {
public:
    LookAheadReader(const StringSlice& input, const wchar_t *sourceFileName);
    ~LookAheadReader();

    const wchar_t *SourceFileName() const { return _sourceFileName; }
    size_t SourceLine() const { return _sourceLine; }
    size_t SourceColumn() const { return _sourceColumn; }

    Location SourceSite() const { return Location(_sourceFileName, _sourceLine, _sourceColumn); }

    bool Eof() const { return _buffer.SizeInBytes() == _bufferOffset; }

    void SeekAbsolute(size_t offset);
    void SeekForward(size_t offset);
    size_t Tell() const {return _bufferOffset; }

    char Read();
    char Peek(size_t n) const;
    void EatWhiteSpaces();

private:
    const wchar_t *_sourceFileName;
    size_t _sourceLine;
    size_t _sourceColumn;

    StringSlice _buffer;
    size_t _bufferOffset;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
