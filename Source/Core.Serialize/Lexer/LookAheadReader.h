#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core/IO/StringView.h"

namespace Core {
class IStreamReader;
enum class ESeekOrigin;
namespace FLexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLookAheadReader {
public:
    STATIC_CONST_INTEGRAL(size_t, BufferCapacity, 2048 - 6 * sizeof(size_t));

    FLookAheadReader(IStreamReader* input, const wchar_t *sourceFileName);
    ~FLookAheadReader();

    const wchar_t *SourceFileName() const { return _sourceFileName; }
    size_t SourceLine() const { return _sourceLine; }
    size_t SourceColumn() const { return _sourceColumn; }

    FLocation SourceSite() const { return FLocation(_sourceFileName, _sourceLine, _sourceColumn); }

    bool Eof() const;
    size_t Tell() const;
    void SeekFwd(size_t off);

    char Peek(size_t n = 0) const;

    char Read();
    bool ReadUntil(FString& dst, char expected);
    bool SkipUntil(char expected);
    void EatWhiteSpaces();

    void Flush();

private:
    const wchar_t *_sourceFileName;
    size_t _sourceLine;
    size_t _sourceColumn;

    IStreamReader* _input;

    size_t _bufferPos;
    size_t _bufferSize;

    u8 _bufferData[BufferCapacity];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace FLexer
} //!namespace Core
