#include "stdafx.h"

#include "Lexer.h"

#include "Symbol.h"
#include "Symbols.h"

#include "Core/Container/Stack.h"
#include "Core/IO/StreamProvider.h"
#include "Core/IO/String.h"
#include "Core/IO/StringBuilder.h"

#include <algorithm>
#include <locale>

namespace Core {
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
// TODO : Minimize indirect writes
// https://www.youtube.com/watch?v=o4-CwDo2zpg
template <size_t _Base>
static void Itoa_(int64_t value, FStringBuilder& str) {
    static_assert(1 < _Base && _Base <= 16, "invalid _Base");
    Assert(str.empty());

    const bool neg = (value < 0);
    value = std::abs(value);

    do {
        const int64_t d = value % _Base;
        const char ch = checked_cast<char>(d + (d > 9 ? ('A'-10) : '0'));

        str.Put(ch);

        value /= _Base;
    } while (value);

    if (neg)
        str.Put('-');

    std::reverse(str.begin(), str.end());

    Assert(str.size());
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
                reader.SeekFwd(toss);
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

    char ch = reader.Peek(0);

    if ('0' == ch)
    {
        if ('x' == ToLower(reader.Peek(1)) )
        {
            // hexadecimal
            ch = reader.Read();
            Assert('0' == ch);

            ch = reader.Read();
            Assert('x' == ch);

            *psymbol = FSymbols::Int;

            if (!ReadCharset_(Hexadecimal_, reader, value))
                CORE_THROW_IT(FLexerException("invalid hexadecimal int", FMatch(FSymbols::Int, value.ToString(), reader.SourceSite(), reader.Tell()) ));

            int64_t numeric;
            if (!Atoi64(&numeric, value.Written(), 16)) {
                Assert(false);
                return false;
            }

            value.clear();
            Itoa_<10>(numeric, value);
            return true;
        }
        else if (IsDigit(reader.Peek(1)) )
        {
            // octal
            ch = reader.Read();
            Assert('0' == ch);

            *psymbol = FSymbols::Int;

            if (!ReadCharset_(Octal_, reader, value))
                CORE_THROW_IT(FLexerException("invalid octal int", FMatch(FSymbols::Int, value.ToString(), reader.SourceSite(), reader.Tell() )));

            int64_t numeric;
            if (!Atoi64(&numeric, value.Written(), 8)) {
                Assert(false);
                return false;
            }

            value.clear();
            Itoa_<10>(numeric, value);
            return true;
        }
    }

    if (Decimal_(ch))
    {
        // decimal or float
        if (!ReadCharset_(Float_, reader, value))
            CORE_THROW_IT(FLexerException("invalid float", FMatch(FSymbols::Float, value.ToString(), reader.SourceSite(), reader.Tell() )));

        Assert(value.size());

        // if '.' found then it is a float
        *psymbol = (value.Written().Contains('.'))
            ? FSymbols::Float
            : FSymbols::Int;

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

    char ch = reader.Peek(0);

    if ('\'' == ch)
    {
        // strong quoting
        ch = reader.Read();
        Assert('\'' == ch);

        *psymbol = FSymbols::String;

        ReadCharset_(Until_<'\''>, reader, value);

        if ('\'' != (ch = reader.Read()))
            CORE_THROW_IT(FLexerException("unterminated strong quoted string", FMatch(FSymbols::String, value.ToString(), reader.SourceSite(), reader.Tell() )));

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
                        CORE_THROW_IT(FLexerException("invalid hexadecimal character escaping", FMatch(FSymbols::String, value.ToString(), reader.SourceSite(), reader.Tell() )));

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
                        CORE_THROW_IT(FLexerException("invalid unicode character escaping", FMatch(FSymbols::String, value.ToString(), reader.SourceSite(), reader.Tell())));

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
                    CORE_THROW_IT(FLexerException("unterminated weak quoted string", FMatch(FSymbols::String, value.ToString(), reader.SourceSite(), reader.Tell() )));

                default:
                    CORE_THROW_IT(FLexerException("invalid character escaping", FMatch(FSymbols::String, value.ToString(), reader.SourceSite(), reader.Tell() )));
                }
            }
            else
            {
                switch (ch)
                {
                case '"': inQuote = false; break;
                case '\\': escaped = true; break;

                case '\0':
                    CORE_THROW_IT(FLexerException("unterminated weak quoted string", FMatch(FSymbols::String, value.ToString(), reader.SourceSite(), reader.Tell() )));

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
FLexer::FLexer(IBufferedStreamReader* input, const FWStringView& sourceFileName, bool allowTypenames)
:   _sourceFileName(sourceFileName.begin(), sourceFileName.end())
,   _reader(input, _sourceFileName)
,   _allowTypenames(allowTypenames)
,   _peeking(false) {
    _lexing.reserve(512);
}
//----------------------------------------------------------------------------
FLexer::~FLexer() {}
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

    match._site = _reader.SourceSite();
    match._offset = _reader.Tell();
    match._value.clear();

    FStringBuilder oss;

    char poken = _reader.Peek(0);
    while ('\0' != poken && ch != poken) {
        oss << _reader.Read();
        poken = _reader.Peek(0);
    }

    if (poken == '\0') {
        match._symbol = FSymbols::Eof;
        return false;
    }
    else {
        Assert(ch == poken);
        match._symbol = FSymbols::String;
        oss.ToString(match._value);
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
        _reader.Reset(_peek.Offset(), _peek.Site());
        _peeking = false;
        _peek = FMatch();
    }

    const size_t offset = _reader.Tell();
    const Lexer::FLocation site = _reader.SourceSite();

    for (;;) {
        char ch = _reader.Peek();
        if (ch == str[0] && ReadIFN(str))
            return true;

        if (_reader.Read() == '\0') {
            _reader.Reset(offset, site);
            return false;
        }
    }

    AssertNotReached();
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

    const size_t offset = _reader.Tell();
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

    _reader.Reset(offset, site);
    return false;
}
//----------------------------------------------------------------------------
bool FLexer::ReadIFN(const Core::Lexer::FSymbol* expected) {
    FMatch match;
    return ReadIFN(match, expected);
}
//----------------------------------------------------------------------------
bool FLexer::ReadIFN(FMatch& match, const Core::Lexer::FSymbol* expected) {
    Assert(expected);
    return (Peek(expected) ? Expect(match, expected) : false);
}
//----------------------------------------------------------------------------
void FLexer::EatWhiteSpaces() {
    RewindPeekIFN();
    _reader.EatWhiteSpaces();
}
//----------------------------------------------------------------------------
bool FLexer::Expect(const Core::Lexer::FSymbol* expected) {
    FMatch match;
    return Expect(match, expected);
}
//----------------------------------------------------------------------------
bool FLexer::Expect(FMatch& match, const Core::Lexer::FSymbol* expected) {
    Assert(expected);
    return (NextMatch_(match) && match.Symbol() == expected);
}
//----------------------------------------------------------------------------
void FLexer::RewindPeekIFN() {
    if (_peeking) {
        _reader.Reset(_peek.Offset(), _peek.Site());
        _peeking = false;
        _peek = FMatch();
    }
}
//----------------------------------------------------------------------------
bool FLexer::NextMatch_(FMatch& match) {
    if (_peeking) {
        _peeking = false;
        match = std::move(_peek);
        _peek = FMatch();
        return (FSymbol::Eof != match.Symbol()->Type());
    }

    _reader.EatWhiteSpaces();

    if ('\0' == _reader.Peek(0)) {
        match._symbol = FSymbols::Eof;
        match._value.clear();
        match._site = _reader.SourceSite();
        match._offset = _reader.Tell();
        return false;
    }

    Lex_Comments_(_reader);

    match._site = _reader.SourceSite();
    match._offset = _reader.Tell();

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

        match._symbol = psymbol;
        _lexing.ToString(match._value);

        _lexing.clear();

        return true;
    }
    else {
        match._symbol = FSymbols::Invalid;
        match._value.clear();

        _lexing.clear();

        return false;
    }
}
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
void FLexerStartup::ClearAll_UnusedMemory() {;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
