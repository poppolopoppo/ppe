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
        ,   _match(std::move(match))
        ,   _site(_match.Site())
    {}

    NODISCARD const Lexer::FMatch& Match() const { return _match; }
    NODISCARD Lexer::FSpan Site() const { return _site; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_SERIALIZE_API virtual FTextWriter& Description(FTextWriter& oss) const override final;
#endif

private:
    Lexer::FMatch _match;
    Lexer::FErrorSpan _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FLexer {
public:
    FLexer(IBufferedStreamReader& input, const FWStringView& sourceFileName, bool allowTypenames);
    ~FLexer();

    NODISCARD const FMatch* Peek();
    NODISCARD const FMatch* Peek(FSymbolRef symbol);

    NODISCARD bool Read(FMatch& match);
    NODISCARD bool ReadUntil(FMatch& match, const char ch);
    NODISCARD bool SkipUntil(const char ch);
    NODISCARD bool SkipUntil(const FStringView& str);
    NODISCARD bool SkipUntil(FStringLiteral literal) { return SkipUntil(literal.MakeView()); }

    NODISCARD bool ReadIFN(char ch, ECase cmp = ECase::Insensitive);
    NODISCARD bool ReadIFN(const FStringView& str, ECase cmp = ECase::Insensitive);
    NODISCARD bool ReadIFN(const PPE::Lexer::FSymbol* expected);
    NODISCARD bool ReadIFN(FMatch& match, FSymbolRef expected);
    NODISCARD bool ReadIFN(FStringLiteral literal, ECase cmp = ECase::Insensitive) { return ReadIFN(literal.MakeView(), cmp); }

    NODISCARD void EatWhiteSpaces();

    NODISCARD bool Expect(FSymbolRef expected);
    NODISCARD bool Expect(FMatch& match, FSymbolRef expected);

    void RewindPeekIFN();

    NODISCARD FWStringView SourceFileName() const { return _sourceFileName; }
    NODISCARD FLocation SourceSite() const { return _reader.SourceSite(); }

private:
    bool NextMatch_(FMatch& match);

    FWStringView _sourceFileName;
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
    PPE_SERIALIZE_API static void Start();
    PPE_SERIALIZE_API static void Shutdown();
    PPE_SERIALIZE_API static void ClearAll_UnusedMemory();

    FLexerStartup() { Start(); }
    ~FLexerStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE
