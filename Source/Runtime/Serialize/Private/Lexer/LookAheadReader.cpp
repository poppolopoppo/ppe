#include "stdafx.h"

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
FLookAheadReader::FLookAheadReader(IBufferedStreamReader* input, const FWStringView& sourceFileName)
:   _sourceFileName(sourceFileName)
,   _sourceLine(1)
,   _sourceColumn(1)
,   _input(input) {
    Assert(input);
}
//----------------------------------------------------------------------------
FLookAheadReader::~FLookAheadReader() {}
//----------------------------------------------------------------------------
bool FLookAheadReader::Eof() const {
    return _input->Eof();
}
//----------------------------------------------------------------------------
size_t FLookAheadReader::Tell() const {
    return checked_cast<size_t>(_input->TellI());
}
//----------------------------------------------------------------------------
void FLookAheadReader::SeekFwd(size_t offset) {
    _input->SeekI(offset, ESeekOrigin::Relative);
}
//----------------------------------------------------------------------------
void FLookAheadReader::Reset(size_t off, const FLocation& site) {
    Assert(site.Filename == _sourceFileName);
    _input->SeekI(off, ESeekOrigin::Begin);
    _sourceLine = site.Line;
    _sourceColumn = site.Column;
}
//----------------------------------------------------------------------------
char FLookAheadReader::Peek(size_t n/* = 0 */) const {
    char ch;
    bool result;
    if (n == 0) {
        result = _input->Peek(ch);
    }
    else {
        const auto origin = _input->TellI();
        _input->SeekI(n, ESeekOrigin::Relative);
        result = _input->Peek(ch);
        _input->SeekI(origin, ESeekOrigin::Begin);
    }
    return (result ? ch : '\0');
}
//----------------------------------------------------------------------------
char FLookAheadReader::Read() {
    char ch;
    if (not _input->ReadPOD(&ch))
        return '\0';

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
    while (char read = Peek()) {
        if (expected == read)
            return true;

        dst += Read();
    }
    return false;
}
//----------------------------------------------------------------------------
bool FLookAheadReader::SkipUntil(char expected) {
    while (char read = Peek()) {
        if (expected == read)
            return true;

        Read();
    }
    return false;
}
//----------------------------------------------------------------------------
void FLookAheadReader::EatWhiteSpaces() {
    while (IsSpace(Peek(0)))
        Read();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE