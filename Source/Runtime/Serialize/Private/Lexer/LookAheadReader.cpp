// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Lexer/LookAheadReader.h"

#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/StringView.h"

#include <locale>

namespace PPE {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLookAheadReader::FLookAheadReader(IBufferedStreamReader& input, const FWStringView& sourceFileName)
:   _sourceFileName(sourceFileName)
,   _sourceLine(1)
,   _sourceColumn(1)
,   _input(input)
{}
//----------------------------------------------------------------------------
FLookAheadReader::~FLookAheadReader() = default;
//----------------------------------------------------------------------------
bool FLookAheadReader::Eof() const {
    return _input.Eof();
}
//----------------------------------------------------------------------------
FLocation FLookAheadReader::SourceSite() const {
    return FLocation(_sourceFileName, _sourceLine, _sourceColumn, _input.TellI());
}
//----------------------------------------------------------------------------
void FLookAheadReader::SkipFwd(size_t offset) {
    while (offset--)
        Unused(Read()); // need to parse source line to keep location coherent with the stream
}
//----------------------------------------------------------------------------
void FLookAheadReader::Reset(const FLocation& site) {
    Assert_NoAssume(site.Filename == _sourceFileName);

    _input.SeekI(site.Offset, ESeekOrigin::Begin);
    _sourceLine = site.Line;
    _sourceColumn = site.Column;
}
//----------------------------------------------------------------------------
Meta::TOptional<char> FLookAheadReader::Peek(size_t n/* = 0 */) const {
    char ch;
    bool result;
    if (n == 0) {
        result = _input.Peek(ch);
    }
    else {
        const auto origin = _input.TellI();
        _input.SeekI(n, ESeekOrigin::Relative);
        result = _input.Peek(ch);
        _input.SeekI(origin, ESeekOrigin::Begin);
    }
    if (result)
        return ch;
    return std::nullopt;
}
//----------------------------------------------------------------------------
Meta::TOptional<char> FLookAheadReader::Read() {
    char ch;
    if (not _input.ReadPOD(&ch))
        return std::nullopt;

    if ('\n' == ch) {
        ++_sourceLine;
        _sourceColumn = 1;
    }
    else {
        ++_sourceColumn;
    }

    return ch;
}
//----------------------------------------------------------------------------
bool FLookAheadReader::ReadUntil(FString& dst, char expected) {
    while (const Meta::TOptional<char> read = Peek()) {
        if (expected == read.value())
            return true;

        dst += Read().value();
    }
    return false;
}
//----------------------------------------------------------------------------
bool FLookAheadReader::SkipUntil(char expected) {
    while (const Meta::TOptional<char> read = Peek()) {
        if (expected == read.value())
            return true;

        Unused(Read()); // need to parse source line to keep location coherent with the stream
    }
    return false;
}
//----------------------------------------------------------------------------
void FLookAheadReader::EatWhiteSpaces() {
    while (IsSpace(Peek(0).value_or('\0')))
        Unused(Read()); // need to parse source line to keep location coherent with the stream
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE
