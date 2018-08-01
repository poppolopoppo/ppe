#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core/IO/StringView.h"
#include "Core/IO/String_fwd.h"

namespace Core {
class IBufferedStreamReader;
enum class ESeekOrigin : size_t;
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLookAheadReader {
public:
    FLookAheadReader(IBufferedStreamReader* input, const FWStringView& sourceFileName);
    ~FLookAheadReader();

    const FWStringView& SourceFileName() const { return _sourceFileName; }
    size_t SourceLine() const { return _sourceLine; }
    size_t SourceColumn() const { return _sourceColumn; }

    FLocation SourceSite() const { return FLocation(_sourceFileName, _sourceLine, _sourceColumn); }

    bool Eof() const;
    size_t Tell() const;
    void SeekFwd(size_t off);
    void Reset(size_t off, const FLocation& site);

    char Peek(size_t n = 0) const;

    char Read();
    bool ReadUntil(FString& dst, char expected);
    bool SkipUntil(char expected);
    void EatWhiteSpaces();

private:
    FWStringView _sourceFileName;
    size_t _sourceLine;
    size_t _sourceColumn;

    IBufferedStreamReader* _input;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
