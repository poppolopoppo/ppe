#include "stdafx.h"

#include "Lexer.h"

#include "Symbol.h"
#include "Symbols.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"
#include "Core/Container/Stack.h"
#include "Core/IO/Stream.h"
#include "Core/IO/String.h"

#include <algorithm>
#include <locale>

namespace Core {
namespace Lexer {
POOL_TAG_DEF(FLexer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ReadCharset_(bool(&charset)(const char), FLookAheadReader& reader, FString& value) {
    Assert(value.empty());

    char ch = reader.Peek(0);
    while (charset(ch)) {
        value += ch;
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
    return ((ch >= '0') && (ch <= '9')) || (ch == '.');
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
template <size_t _Base>
static void Itoa_(int64_t value, FString& str) {
    static_assert(1 < _Base && _Base <= 16, "invalid _Base");
    Assert(str.empty());

    const bool neg = (value < 0);
    value = std::abs(value);

    do {
        const int64_t d = value % _Base;
        const char ch = checked_cast<char>(d + (d > 9 ? ('A'-10) : '0'));

        str += ch;

        value /= _Base;
    } while (value);

    if (neg)
        str += '-';

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

    while ('/' == ch && '/' == reader.Peek(1))
    {
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

    const FSymbols& symbols = FSymbols::Instance();

    STACKLOCAL_POD_STACK(char, poken, FSymbols::MaxLength);

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
static bool Lex_Numeric_(FLookAheadReader& reader, const FSymbol **psymbol, FString& value) {
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
                CORE_THROW_IT(FLexerException("invalid hexadecimal int", FMatch(FSymbols::Int, std::move(value), reader.SourceSite(), reader.Tell()) ));

            int64_t numeric;
            if (!Atoi64(&numeric, MakeStringView(value), 16)) {
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
                CORE_THROW_IT(FLexerException("invalid octal int", FMatch(FSymbols::Int, std::move(value), reader.SourceSite(), reader.Tell() )));

            int64_t numeric;
            if (!Atoi64(&numeric, MakeStringView(value), 8)) {
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
            CORE_THROW_IT(FLexerException("invalid float", FMatch(FSymbols::Float, std::move(value), reader.SourceSite(), reader.Tell() )));

        Assert(value.size());

        // if '.' found then it is a float
        const size_t n = value.find_first_of('.');
        *psymbol = (n < value.size())
            ? FSymbols::Float
            : FSymbols::Int;

        return true;
    }

    *psymbol = FSymbols::Invalid;
    Assert(value.empty());

    return false;
}
//----------------------------------------------------------------------------
static bool Lex_String_(FLookAheadReader& reader, const FSymbol **psymbol, FString& value) {
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
            CORE_THROW_IT(FLexerException("unterminated strong quoted string", FMatch(FSymbols::String, std::move(value), reader.SourceSite(), reader.Tell() )));

        return true;
    }
    else if ('"' == ch)
    {
        FOStringStream oss;

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
                case '0': oss.put('\0'); break;

                case 'a': oss.put('\a'); break;
                case 'b': oss.put('\b'); break;
                case 'f': oss.put('\f'); break;
                case 'n': oss.put('\n'); break;
                case 'r': oss.put('\r'); break;
                case 't': oss.put('\t'); break;

                case '"': oss.put('"'); break;
                case '\'': oss.put('\''); break;
                case '\\': oss.put('\\'); break;

                case 'o':
                    d0 = ToLower(reader.Read());
                    d1 = ToLower(reader.Read());

                    if (!(Octal_(d0) && Octal_(d1)))
                        CORE_THROW_IT(FLexerException("invalid octal character escaping", FMatch(FSymbols::String, std::move(value), reader.SourceSite(), reader.Tell() )));

                    ch = (char)((d0 - '0') * 8 + (d1 - '0') );

                    oss.put(ch);

                    break;

                case 'x':
                    d0 = ToLower(reader.Read());
                    d1 = ToLower(reader.Read());

                    if (!(Hexadecimal_(d0) && Hexadecimal_(d1)))
                        CORE_THROW_IT(FLexerException("invalid hexadecimal character escaping", FMatch(FSymbols::String, std::move(value), reader.SourceSite(), reader.Tell() )));

                    ch = (char)((d0 <= '9' ? d0 - '0' : d0 - 'a') * 16 +
                                (d1 <= '9' ? d1 - '0' : d1 - 'a') );

                    oss.put(ch);

                    break;

                case 'u':
                    d0 = ToLower(reader.Read());
                    d1 = ToLower(reader.Read());
                    d2 = ToLower(reader.Read());
                    d3 = ToLower(reader.Read());

                    if (!(Hexadecimal_(d0) && Hexadecimal_(d1) && Hexadecimal_(d2) && Hexadecimal_(d3)))
                        CORE_THROW_IT(FLexerException("invalid unicode character escaping", FMatch(FSymbols::String, std::move(value), reader.SourceSite(), reader.Tell())));

                    unicode =   u16(d0 <= '9' ? d0 - '0' : d0 - 'a') * (16 * 16 * 16) +
                                u16(d1 <= '9' ? d1 - '0' : d1 - 'a') * (16 * 16) +
                                u16(d2 <= '9' ? d2 - '0' : d2 - 'a') * (16) +
                                u16(d3 <= '9' ? d3 - '0' : d3 - 'a');

                    if (unicode <= 0xFF) {
                        oss.put(char(unicode));
                    }
                    else {
                        oss.put(char(unicode >> 8));
                        oss.put(char(unicode & 0xFF));
                    }

                    break;

                case '\0':
                    CORE_THROW_IT(FLexerException("unterminated weak quoted string", FMatch(FSymbols::String, std::move(value), reader.SourceSite(), reader.Tell() )));

                default:
                    CORE_THROW_IT(FLexerException("invalid character escaping", FMatch(FSymbols::String, std::move(value), reader.SourceSite(), reader.Tell() )));
                }
            }
            else
            {
                switch (ch)
                {
                case '"': inQuote = false; break;
                case '\\': escaped = true; break;

                case '\0':
                    CORE_THROW_IT(FLexerException("unterminated weak quoted string", FMatch(FSymbols::String, std::move(value), reader.SourceSite(), reader.Tell() )));

                default:
                    oss.put(ch);
                    break;
                }
            }
        } while (inQuote);

        value = oss.str();
        return true;
    }

    *psymbol = FSymbols::Invalid;
    Assert(value.empty());

    return false;
}
//----------------------------------------------------------------------------
static bool Lex_Identifier_(FLookAheadReader& reader, const FSymbol **psymbol, FString& value) {
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
FLexer::FLexer(IStreamReader* input, const FWStringView& sourceFileName, bool allowTypenames)
:   _sourceFileName(sourceFileName.begin(), sourceFileName.end())
,   _reader(input, _sourceFileName.c_str())
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

    if (_peeking) {
        _peeking = false;
        _peek = FMatch();
    }

    _reader.EatWhiteSpaces();

    match._site = _reader.SourceSite();
    match._offset = _reader.Tell();
    match._value.clear();

    FOStringStream oss;

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
        match._value = oss.str();
        return true;
    }
}
//----------------------------------------------------------------------------
bool FLexer::SkipUntil(const char ch) {
    Assert('\0' != ch);

    if (_peeking) {
        _peeking = false;
        _peek = FMatch();
    }

    char poken = _reader.Peek(0);
    while (poken && ch != poken)
        _reader.Read();

    return (ch == poken);
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
        match._value = std::move(_lexing);

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
    POOL_TAG(FLexer)::Start();
    FSymbols::Create();
}
//----------------------------------------------------------------------------
void FLexerStartup::Shutdown() {
    FSymbols::Destroy();
    POOL_TAG(FLexer)::Shutdown();
}
//----------------------------------------------------------------------------
void FLexerStartup::ClearAll_UnusedMemory() {
    POOL_TAG(FLexer)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
