#pragma once

#include "Serialize.h"

#include "Lexer/LookAheadReader.h"
#include "Lexer/Match.h"
#include "SerializeExceptions.h"

#include "IO/String.h"
#include "IO/StringBuilder.h"

#include <stdexcept>

namespace PPE {
class IBufferedStreamReader;
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLexerException : public PPE::Serialize::FSerializeException {
public:
    typedef PPE::Serialize::FSerializeException parent_type;

    FLexerException(const char *what, FMatch&& match)
        :   parent_type(what)
        ,   _match(std::move(match)) {}

    virtual ~FLexerException() {}

    const PPE::Lexer::FMatch& Match() const { return _match; }

private:
    PPE::Lexer::FMatch _match;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLexer {
public:
    FLexer(IBufferedStreamReader* input, const FWStringView& sourceFileName, bool allowTypenames);
    ~FLexer();

    const FMatch* Peek();
    const FMatch* Peek(const FSymbol* symbol);

    bool Read(FMatch& match);
    bool ReadUntil(FMatch& match, const char ch);
    bool SkipUntil(const char ch);
    bool SkipUntil(const FStringView& str);

    bool ReadIFN(char ch, ECase cmp = ECase::Insensitive);
    bool ReadIFN(const FStringView& str, ECase cmp = ECase::Insensitive);
    bool ReadIFN(const PPE::Lexer::FSymbol* expected);
    bool ReadIFN(FMatch& match, const PPE::Lexer::FSymbol* expected);

    void EatWhiteSpaces();

    bool Expect(const PPE::Lexer::FSymbol* expected);
    bool Expect(FMatch& match, const PPE::Lexer::FSymbol* expected);

    void RewindPeekIFN();

    const FWString& SourceFileName() const { return _sourceFileName; }
    FLocation SourceSite() const { return _reader.SourceSite(); }

private:
    bool NextMatch_(FMatch& match);

    FWString _sourceFileName;
    FLookAheadReader _reader;

    FStringBuilder _lexing;
    FMatch _peek;

    bool _allowTypenames;
    bool _peeking;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLexerStartup {
    static void Start();
    static void Shutdown();
    static void ClearAll_UnusedMemory();

    FLexerStartup() { Start(); }
    ~FLexerStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE
