#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "IO/StringView.h"
#include "IO/String_fwd.h"

namespace PPE {
class IBufferedStreamReader;
enum class ESeekOrigin : size_t;
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLookAheadReader {
public:
    FLookAheadReader(IBufferedStreamReader& input, const FWStringView& sourceFileName);
    ~FLookAheadReader();

    const FWStringView& SourceFileName() const { return _sourceFileName; }
    size_t SourceLine() const { return _sourceLine; }
    size_t SourceColumn() const { return _sourceColumn; }

    FLocation SourceSite() const;

    bool Eof() const;
    void SkipFwd(size_t offset);
    void Reset(const FLocation& site);

    char Peek(size_t n = 0) const;

    char Read();
    bool ReadUntil(FString& dst, char expected);
    bool SkipUntil(char expected);
    void EatWhiteSpaces();

private:
    FWStringView _sourceFileName;
    size_t _sourceLine;
    size_t _sourceColumn;

    IBufferedStreamReader& _input;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE
