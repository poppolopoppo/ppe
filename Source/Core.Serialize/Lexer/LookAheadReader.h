#pragma once

#include "Core/Core.h"

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
    LookAheadReader(const StringSlice& input, const char *sourceFileName);
    ~LookAheadReader();

    const char *SourceFileName() const { return _sourceFileName; }
    size_t SourceLine() const { return _sourceLine; }
    size_t SourceColumn() const { return _sourceColumn; }

    Location SourceSite() const { return Location(_sourceFileName, _sourceLine, _sourceColumn); }

    bool Eof() const { return _buffer.SizeInBytes() == _bufferOffset; }

    void SeekForward(size_t offset);
    char Read();
    char Peek(size_t n) const;
    void EatWhiteSpaces();

private:
    const char *_sourceFileName;
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
