#pragma once

#include "Core/Core.h"

#include "Core/IO/StringSlice.h"
#include "Core/Lexer/Location.h"

namespace Core {
class IVirtualFileSystemIStream;

namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LookAheadReader {
public:
    enum : size_t {
        BufferCapacity = 1024,
        BufferMask = (BufferCapacity - 1),
        HalfBufferCapacity = (BufferCapacity / 2),
        MaxWordLength = HalfBufferCapacity
    };

    LookAheadReader(const StringSlice& input, const char *sourceFileName);
    LookAheadReader(IVirtualFileSystemIStream *stream, const char *sourceFileName);
    ~LookAheadReader();

    const char *SourceFileName() const { return _sourceFileName; }
    size_t SourceLine() const { return _sourceLine; }
    size_t SourceColumn() const { return _sourceColumn; }

    Location SourceSite() const { return Location(_sourceFileName, _sourceLine, _sourceColumn); }

    bool Eof() const { return _eof; }

    void SeekForward(size_t offset);
    char Read();
    char Peek(size_t n) const;
    void EatWhiteSpaces();

private:
    void RefillBuffer_(size_t index, size_t length);

    IVirtualFileSystemIStream *_stream;

    const char *_sourceFileName;
    size_t _sourceLine;
    size_t _sourceColumn;

    char _buffer[BufferCapacity];
    size_t _bufferOffset;
    size_t _bufferSize;

    bool _eof;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
