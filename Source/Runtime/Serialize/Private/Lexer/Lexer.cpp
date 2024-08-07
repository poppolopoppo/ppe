﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Lexer/Lexer.h"

#include "Lexer/Symbol.h"
#include "Lexer/Symbols.h"

#include "Container/Stack.h"
#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"

#include <algorithm>

namespace PPE {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ReadCharset_(bool(&charset)(const char), FLookAheadReader& reader, FStringBuilder& value) {
    Assert(value.empty());

    char ch = reader.Peek(0);
    while (charset(ch)) {
        value.Put(ch);
        reader.Read();
        ch = reader.Peek(0);
    }

    return 0 != value.size();
}
//----------------------------------------------------------------------------
static bool Octal_(const char ch) {
    return ((ch >= '0') && (ch <= '7'));
}
//----------------------------------------------------------------------------
static bool Decimal_(const char ch) {
    return ((ch >= '0') && (ch <= '9'));
}
//----------------------------------------------------------------------------
static bool Hexadecimal_(const char ch) {
    return ((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F'));
}
//----------------------------------------------------------------------------
static bool Float_(const char ch) {
    return ((ch >= '0') && (ch <= '9')) || (ch == '.') || (ch == 'e') || (ch == 'E');
}
//----------------------------------------------------------------------------
static bool Identifier_(const char ch) {
    return IsAlnum(ch) || (ch == '_');
}
//----------------------------------------------------------------------------
template <char _Ch>
static bool Until_(const char ch) {
    return '\0' != ch && _Ch != ch;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool IsTokenChar_(char ch) {
    return IsAlnum(ch) || '_' == ch;
}
//----------------------------------------------------------------------------
static void Lex_Comments_(FLookAheadReader& reader) {
    char ch = reader.Peek(0);

    while ('/' == ch && '/' == reader.Peek(1)) {
        char r = reader.Read();
        Assert('/' == r);

        r = reader.Read();
        Assert('/' == r);

        while ('\n' != r && '\0' != r)
            r = reader.Read();

        reader.EatWhiteSpaces();

        ch = reader.Peek(0);
    }
}
//----------------------------------------------------------------------------
static bool Lex_Symbol_(FLookAheadReader& reader, const FSymbol **psymbol, bool allowTypenames) {
    Assert(psymbol);
    Assert(FSymbols::Invalid == *psymbol);

    size_t offset = 0;
    size_t toss = 0;

    const FSymbols& symbols = FSymbols::Get();

    TFixedSizeStack<char, FSymbols::MaxLength> poken;

    const FSymbol* result = nullptr;
    do {
        poken.Push(reader.Peek(offset++));
        if (symbols.IsPrefix(&result, MakeConstView(poken))) {
            Assert(result);
            Assert(FSymbol::Invalid != result->Type());
            if (result->IsValid()) {
                *psymbol = result;
                toss = offset;
            }
            else {
                Assert(result->IsPrefix());
            }
        }
        else {
            Assert(nullptr == result);
        }
    }
    while (result);

    if (0 != toss) {
        Assert(*psymbol);
        Assert((*psymbol)->Type() != FSymbol::Invalid);

        const char ch0 = reader.Peek(toss - 1);
        const char ch1 = reader.Peek(toss);
        if (IsTokenChar_(ch0) && IsTokenChar_(ch1)) {
            // incomplete token
            *psymbol = FSymbols::Invalid;
            return false;
        }
        else {
            if ((*psymbol)->Type() == FSymbol::Typename &&
                not allowTypenames ) {
                *psymbol = FSymbols::Invalid;
                return false;
            }
            else {
                Assert(FSymbols::Invalid != *psymbol);
                reader.SkipFwd(toss);
                return true;
            }
        }
    }
    else {
        Assert(FSymbols::Invalid == *psymbol);
        return false;
    }
}
//----------------------------------------------------------------------------
static bool Lex_Numeric_(FLookAheadReader& reader, const FSymbol **psymbol, FStringBuilder& value) {
    Assert(psymbol);
    Assert(value.empty());

    const FLocation origin = reader.SourceSite();

    char ch = reader.Peek(0);

    if ('0' == ch)
    {
        if ('x' == ToLower(reader.Peek(1)) )
        {
            // hexadecimal
            ch = reader.Read();
            Assert('0' == ch);

            ch = reader.Read();
            Assert_NoAssume('x' == ToLower(ch));

            if (!ReadCharset_(Hexadecimal_, reader, value))
                PPE_THROW_IT(FLexerException("invalid hexadecimal int", FMatch(FSymbols::Integer, value.ToString(), origin, reader.SourceSite())));

            if ('u' == ToLower(reader.Peek())) {
                ch = reader.Read();
                Assert_NoAssume('u' == ToLower(ch));

                uint64_t u;
                if (!Atoi(&u, value.Written(), 16)) {
                    return false;
                }

                // reformat as base 10 unsigned int
                value.clear();
                value << u;
                *psymbol = FSymbols::Unsigned;
            }
            else {
                int64_t i;
                if (!Atoi(&i, value.Written(), 16)) {
                    return false;
                }

                // reformat as base 10 signed int
                value.clear();
                value << i;
                *psymbol = FSymbols::Integer;
            }

            return true;
        }
        else if (IsDigit(reader.Peek(1)) )
        {
            // octal
            ch = reader.Read();
            Assert('0' == ch);

            if (!ReadCharset_(Octal_, reader, value))
                PPE_THROW_IT(FLexerException("invalid octal int", FMatch(FSymbols::Integer, value.ToString(), origin, reader.SourceSite())));

            if ('u' == ToLower(reader.Peek())) {
                ch = reader.Read();
                Assert_NoAssume('u' == ToLower(ch));

                uint64_t u;
                if (!Atoi(&u, value.Written(), 8)) {
                    return false;
                }

                // reformat as base 10 unsigned int
                value.clear();
                value << u;
                *psymbol = FSymbols::Unsigned;
            }
            else {
                int64_t i;
                if (!Atoi(&i, value.Written(), 8)) {
                    return false;
                }

                // reformat as base 10 signed int
                value.clear();
                value << i;
                *psymbol = FSymbols::Integer;
            }

            return true;
        }
    }

    if (Decimal_(ch))
    {
        // decimal or float
        if (!ReadCharset_(Float_, reader, value))
            PPE_THROW_IT(FLexerException("invalid float", FMatch(FSymbols::Float, value.ToString(), origin, reader.SourceSite())));

        Assert(value.size());

        // if '.' found then it is a float
        int is_float = 0;
        for (char c : value.Written()) {
            if (c == '.' && ++is_float > 1) // can't have several '.'
                PPE_THROW_IT(FLexerException("float can have only one decimal separator", FMatch(FSymbols::Float, value.ToString(), origin, reader.SourceSite())));
        }

        if ('u' == ToLower(reader.Peek())) {
            ch = reader.Read();
            Assert_NoAssume('u' == ToLower(ch));

            if (is_float)
                PPE_THROW_IT(FLexerException("float can't be unsigned", FMatch(FSymbols::Float, value.ToString(), origin, reader.SourceSite())));

            *psymbol = FSymbols::Unsigned;
        }
        else {
            *psymbol = (is_float ? FSymbols::Float : FSymbols::Integer);
        }

        return true;
    }

    *psymbol = FSymbols::Invalid;
    Assert(value.empty());

    return false;
}
//----------------------------------------------------------------------------
static bool Lex_String_(FLookAheadReader& reader, const FSymbol **psymbol, FStringBuilder& value) {
    Assert(psymbol);
    Assert(value.empty());

    const FLocation origin = reader.SourceSite();

    char ch = reader.Peek(0);

    if ('\'' == ch)
    {
        // strong quoting
        ch = reader.Read();
        Assert('\'' == ch);

        *psymbol = FSymbols::String;

        ReadCharset_(Until_<'\''>, reader, value);

        if ('\'' != (ch = reader.Read()))
            PPE_THROW_IT(FLexerException("unterminated strong quoted string", FMatch(FSymbols::String, value.ToString(), origin, reader.SourceSite())));

        return true;
    }
    else if ('"' == ch)
    {
        FStringBuilder& oss = value;

        // weak quoting
        ch = reader.Read();
        Assert('"' == ch);

        *psymbol = FSymbols::String;

        bool inQuote = true;
        bool escaped = false;
        do
        {
            ch = reader.Read();

            if (escaped)
            {
                escaped = false;

                char d0, d1, d2, d3;
                u16 unicode;

                // http://en.wikipedia.org/wiki/Escape_sequences_in_C
                switch (ToLower(ch))
                {
                case 'a': oss.Put('\a'); break;
                case 'b': oss.Put('\b'); break;
                case 'f': oss.Put('\f'); break;
                case 'n': oss.Put('\n'); break;
                case 'r': oss.Put('\r'); break;
                case 't': oss.Put('\t'); break;

                case '"': oss.Put('"'); break;
                case '\'': oss.Put('\''); break;
                case '\\': oss.Put('\\'); break;

                // octal
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    d0 = u8(ch - '0');
                    forrange(i, 0, 3) {
                        ch = reader.Read();
                        if (not Octal_(ch))
                            break;

                        d0 = d0 * 8 + (ch - '0');
                    }

                    oss.Put(char(d0));

                    break;

                // hexadecimal
                case 'x':
                    d0 = ToLower(reader.Read());
                    d1 = ToLower(reader.Read());

                    if (!(Hexadecimal_(d0) && Hexadecimal_(d1)))
                        PPE_THROW_IT(FLexerException("invalid hexadecimal character escaping", FMatch(FSymbols::String, value.ToString(), origin, reader.SourceSite())));

                    ch = (char)((d0 <= '9' ? d0 - '0' : d0 - 'a' + 10) * 16 +
                                (d1 <= '9' ? d1 - '0' : d1 - 'a' + 10) );

                    oss.Put(ch);

                    break;

                // unicode
                case 'u':
                    d0 = ToLower(reader.Read());
                    d1 = ToLower(reader.Read());
                    d2 = ToLower(reader.Read());
                    d3 = ToLower(reader.Read());

                    if (!(Hexadecimal_(d0) && Hexadecimal_(d1) && Hexadecimal_(d2) && Hexadecimal_(d3)))
                        PPE_THROW_IT(FLexerException("invalid unicode character escaping", FMatch(FSymbols::String, value.ToString(), origin, reader.SourceSite())));

                    unicode =   u16(d0 <= '9' ? d0 - '0' : d0 - 'a' + 10) * (16 * 16 * 16) +
                                u16(d1 <= '9' ? d1 - '0' : d1 - 'a' + 10) * (16 * 16) +
                                u16(d2 <= '9' ? d2 - '0' : d2 - 'a' + 10) * (16) +
                                u16(d3 <= '9' ? d3 - '0' : d3 - 'a' + 10);

                    if (unicode <= 0xFF) {
                        oss.Put(char(unicode));
                    }
                    else {
                        oss.Put(char(unicode >> 8));
                        oss.Put(char(unicode & 0xFF));
                    }

                    break;

                case '\0':
                    PPE_THROW_IT(FLexerException("unterminated weak quoted string", FMatch(FSymbols::String, value.ToString(), origin, reader.SourceSite())));

                default:
                    //PPE_THROW_IT(FLexerException("invalid character escaping", FMatch(FSymbols::String, value.ToString(), reader.SourceSite())));
                    oss.Put('\\'); // put both chars and ignore escaping
                    oss.Put(ch);
                    break;
                }
            }
            else
            {
                switch (ch)
                {
                case '"': inQuote = false; break;
                case '\\': escaped = true; break;

                case '\0':
                    PPE_THROW_IT(FLexerException("unterminated weak quoted string", FMatch(FSymbols::String, value.ToString(), origin, reader.SourceSite())));

                default:
                    oss.Put(ch);
                    break;
                }
            }
        } while (inQuote);

        return true;
    }

    *psymbol = FSymbols::Invalid;
    Assert(value.empty());

    return false;
}
//----------------------------------------------------------------------------
static bool Lex_Identifier_(FLookAheadReader& reader, const FSymbol **psymbol, FStringBuilder& value) {
    Assert(psymbol);
    Assert(value.empty());

    char ch = reader.Peek(0);

    if (IsAlpha(ch) || ('_' == ch))
    {
        *psymbol = FSymbols::Identifier;
        return ReadCharset_(Identifier_, reader, value);
    }

    *psymbol = FSymbols::Invalid;
    Assert(value.empty());

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLexer::FLexer(IBufferedStreamReader& input, const FWStringView& sourceFileName, bool allowTypenames)
:   _sourceFileName(sourceFileName.begin(), sourceFileName.end())
,   _reader(input, _sourceFileName)
,   _allowTypenames(allowTypenames)
,   _peeking(false) {
    _lexing.reserve(512);
    _lexing << FTextFormat::Decimal;
}
//----------------------------------------------------------------------------
FLexer::~FLexer() {
    Assert(_lexing.Format().Base() == FTextFormat::Decimal);
}
//----------------------------------------------------------------------------
const FMatch *FLexer::Peek() {
    if (!_peeking) {
        Assert(!_peek.Valid());
        _peeking = NextMatch_(_peek);
    }
    return ((_peek.Valid()) ? &_peek : nullptr);
}
//----------------------------------------------------------------------------
const FMatch* FLexer::Peek(const FSymbol* symbol) {
    Assert(symbol);
    const FMatch* poken = Peek();
    return (poken && poken->Symbol() == symbol) ? poken : nullptr;
}
//----------------------------------------------------------------------------
bool FLexer::Read(FMatch& match) {
    return NextMatch_(match);
}
//----------------------------------------------------------------------------
bool FLexer::ReadUntil(FMatch& match, const char ch) {
    Assert('\0' != ch);
    AssertRelease(not _peeking); // because we can't rewind the stream

    _reader.EatWhiteSpaces();

    const FLocation origin = _reader.SourceSite();

    FStringBuilder oss;
    char poken = _reader.Peek(0);
    while ('\0' != poken && ch != poken) {
        oss << _reader.Read();
        poken = _reader.Peek(0);
    }

    if (poken == '\0') {
        match = FMatch(FSymbols::Eof, FString{}, origin, _reader.SourceSite());
        return false;
    }
    else {
        Assert(ch == poken);
        match = FMatch(FSymbols::Eof, oss.ToString(), origin, _reader.SourceSite());
        return true;
    }
}
//----------------------------------------------------------------------------
bool FLexer::SkipUntil(const char ch) {
    Assert('\0' != ch);

    RewindPeekIFN();

    char poken = _reader.Peek(0);
    while (poken && ch != poken) {
        _reader.Read();
        poken = _reader.Peek(0);
    }

    return (ch == poken);
}
//----------------------------------------------------------------------------
bool FLexer::SkipUntil(const FStringView& str) {
    Assert(not str.empty());

    if (_peeking) {
        _reader.Reset(_peek.Site());
        _peeking = false;
        _peek = FMatch();
    }

    const Lexer::FLocation site = _reader.SourceSite();

    for (;;) {
        char ch = _reader.Peek();
        if (ch == str[0] && ReadIFN(str))
            return true;

        if (_reader.Read() == '\0') {
            _reader.Reset(site);
            return false;
        }
    }
}
//----------------------------------------------------------------------------
bool FLexer::ReadIFN(char ch, ECase cmp/* = ECase::Insensitive */) {
    Assert('\0' != ch);

    RewindPeekIFN();

    char poken = _reader.Peek(0);
    if (not Equals(poken, ch, cmp))
        return false;

    _reader.Read();
    return true;
}
//----------------------------------------------------------------------------
bool FLexer::ReadIFN(const FStringView& str, ECase cmp/* = ECase::Insensitive */) {
    Assert(not str.empty());

    RewindPeekIFN();

    const Lexer::FLocation site = _reader.SourceSite();

    bool match = true;
    forrange(i, 0, str.size()) {
        if (not Equals(_reader.Read(), str[i], cmp)) {
            match = false;
            break;
        }
    }

    if (match)
        return true;

    _reader.Reset(site);
    return false;
}
//----------------------------------------------------------------------------
bool FLexer::ReadIFN(const PPE::Lexer::FSymbol* expected) {
    FMatch match;
    return ReadIFN(match, expected);
}
//----------------------------------------------------------------------------
bool FLexer::ReadIFN(FMatch& match, const PPE::Lexer::FSymbol* expected) {
    Assert(expected);
    return (Peek(expected) ? Expect(match, expected) : false);
}
//----------------------------------------------------------------------------
void FLexer::EatWhiteSpaces() {
    RewindPeekIFN();
    _reader.EatWhiteSpaces();
}
//----------------------------------------------------------------------------
bool FLexer::Expect(const PPE::Lexer::FSymbol* expected) {
    FMatch match;
    return Expect(match, expected);
}
//----------------------------------------------------------------------------
bool FLexer::Expect(FMatch& match, const PPE::Lexer::FSymbol* expected) {
    Assert(expected);
    return (NextMatch_(match) && match.Symbol() == expected);
}
//----------------------------------------------------------------------------
void FLexer::RewindPeekIFN() {
    if (_peeking) {
        _reader.Reset(_peek.Site());
        _peeking = false;
        _peek = FMatch();
    }
}
//----------------------------------------------------------------------------
bool FLexer::NextMatch_(FMatch& match) {
    if (_peeking) {
        _peeking = false;
        match = std::move(_peek);
        return (FSymbol::Eof != match.Symbol()->Type());
    }

    _reader.EatWhiteSpaces();

    FLocation origin = _reader.SourceSite();

    if ('\0' == _reader.Peek(0)) {
        match = FMatch(FSymbols::Eof, FString{}, origin, _reader.SourceSite());
        return false;
    }

    Lex_Comments_(_reader);

    origin = _reader.SourceSite();

    const FSymbol *psymbol = nullptr;
    Assert(_lexing.empty());

    // by priority order :
    if (Lex_Numeric_(_reader, &psymbol, _lexing) ||
        Lex_String_(_reader, &psymbol, _lexing) ||
        Lex_Symbol_(_reader, &psymbol, _allowTypenames) ||
        Lex_Identifier_(_reader, &psymbol, _lexing)
        ) {
        Assert(psymbol);
        Assert(FSymbol::Eof != psymbol->Type());
        Assert(FSymbol::Invalid != psymbol->Type());

        match = FMatch(psymbol, _lexing.ToString(), origin, _reader.SourceSite());
        Assert_NoAssume(_lexing.empty()); // should be cleared inside ToString()

        return true;
    }
    else {
        match = FMatch(FSymbols::Invalid, FString{}, origin, _reader.SourceSite());
        _lexing.clear();

        return false;
    }
}
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FLexerException::Description(FTextWriter& oss) const {
    return oss
        << MakeCStringView(What())
        << ": with token ["
        << Match()
        << "] at '"
        << Site()
        << "' !";
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLexerStartup::Start() {
    FSymbols::Create();
}
//----------------------------------------------------------------------------
void FLexerStartup::Shutdown() {
    FSymbols::Destroy();
}
//----------------------------------------------------------------------------
void FLexerStartup::ClearAll_UnusedMemory() {
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE
