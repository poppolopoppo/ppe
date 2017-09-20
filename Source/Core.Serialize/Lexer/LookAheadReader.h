#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core/IO/StringView.h"

namespace Core {
class IBufferedStreamReader;
enum class ESeekOrigin;
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLookAheadReader {
public:
    FLookAheadReader(IBufferedStreamReader* input, const wchar_t *sourceFileName);
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

private:
    const wchar_t *_sourceFileName;
    size_t _sourceLine;
    size_t _sourceColumn;

    IBufferedStreamReader* _input;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
