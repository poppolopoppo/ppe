#include "stdafx.h"

#include "Lexer.h"

#include "Symbol.h"
#include "SymbolTrie.h"

#include "Core/IO/String.h"

#include <algorithm>
#include <locale>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ReadCharset_(bool(&charset)(const char), LookAheadReader& reader, String& value) {
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
static void Itoa_(int64_t value, String& str) {
    static_assert(1 < _Base && _Base <= 16, "invalid _Base");
    Assert(str.empty());

    static const char BASE_FMT[16+1] = "0123456789ABCDEF";

    const bool neg = (value < 0);
    value = std::abs(value);

    do {
        const int64_t d = value % _Base;
        const char ch = BASE_FMT[d];

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
static void Lex_Comments_(LookAheadReader& reader) {
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
static bool Lex_Symbol_(LookAheadReader& reader, const Symbol **psymbol) {
    Assert(psymbol);
    Assert(SymbolTrie::Invalid == *psymbol);

    size_t offset = 0;
    size_t toss = 0;

    SymbolTrie::query_t result;
    do {
        const char ch = reader.Peek(offset++);
        result = SymbolTrie::IsPrefix(ch, result);
        if (result.Node && result.Node->HasValue()) {
            *psymbol = &result.Node->Value();
            Assert((*psymbol)->Type() != Symbol::Invalid);
            toss = offset;
        }
    }
    while (nullptr != result.Node);

    if (0 != toss) {
        Assert(*psymbol);
        Assert((*psymbol)->Type() != Symbol::Invalid);
        reader.SeekForward(toss);
        return true;
    }
    else {
        Assert(SymbolTrie::Invalid == *psymbol);
        return false;
    }
}
//----------------------------------------------------------------------------
static bool Lex_Numeric_(LookAheadReader& reader, const Symbol **psymbol, String& value) {
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

            *psymbol = SymbolTrie::Int;

            if (!ReadCharset_(Hexadecimal_, reader, value))
                throw LexerException("invalid hexadecimal int", Match(SymbolTrie::Int, std::move(value), reader.SourceSite()));

            int64_t numeric;
            if (!Atoi<16>(&numeric, value)) {
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

            *psymbol = SymbolTrie::Int;

            if (!ReadCharset_(Octal_, reader, value))
                throw LexerException("invalid octal int", Match(SymbolTrie::Int, std::move(value), reader.SourceSite()));

            int64_t numeric;
            if (!Atoi<8>(&numeric, value)) {
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
            throw LexerException("invalid float", Match(SymbolTrie::Float, std::move(value), reader.SourceSite()));

        Assert(value.size());

        // if '.' found then it is a float
        const size_t n = value.find_first_of('.');
        *psymbol = (n < value.size())
            ? SymbolTrie::Float
            : SymbolTrie::Int;

        return true;
    }

    *psymbol = SymbolTrie::Invalid;
    Assert(value.empty());

    return false;
}
//----------------------------------------------------------------------------
static bool Lex_String_(LookAheadReader& reader, const Symbol **psymbol, String& value) {
    Assert(psymbol);
    Assert(value.empty());

    char ch = reader.Peek(0);

    if ('\'' == ch)
    {
        // strong quoting
        ch = reader.Read();
        Assert('\'' == ch);

        *psymbol = SymbolTrie::String;

        ReadCharset_(Until_<'\''>, reader, value);

        if ('\'' != (ch = reader.Read()))
            throw LexerException("unterminated strong quoted string", Match(SymbolTrie::String, std::move(value), reader.SourceSite()));

        return true;
    }
    else if ('"' == ch)
    {
        // weak quoting
        ch = reader.Read();
        Assert('"' == ch);

        *psymbol = SymbolTrie::String;

        bool inQuote = true;
        bool escaped = false;
        do
        {
            ch = reader.Read();

            if (escaped)
            {
                escaped = false;

                char d0, d1;

                // http://en.wikipedia.org/wiki/Escape_sequences_in_C
                switch (ToLower(ch))
                {
                case '0': value += '\0'; break;

                case 'a': value += '\a'; break;
                case 'b': value += '\b'; break;
                case 'f': value += '\f'; break;
                case 'n': value += '\n'; break;
                case 'r': value += '\r'; break;
                case 't': value += '\t'; break;

                case '"': value += '"'; break;
                case '\'': value += '\''; break;
                case '\\': value += '\\'; break;

                case 'o':
                    d0 = ToLower(reader.Read());
                    d1 = ToLower(reader.Read());

                    if (!(Octal_(d0) && Octal_(d1)))
                        throw LexerException("invalid octal character escaping", Match(SymbolTrie::String, std::move(value), reader.SourceSite()));

                    ch = (char)((d0 - '0') * 8 + (d1 - '0') );

                    value += ch;

                    break;

                case 'x':
                    d0 = ToLower(reader.Read());
                    d1 = ToLower(reader.Read());

                    if (!(Hexadecimal_(d0) && Hexadecimal_(d1)))
                        throw LexerException("invalid hexadecimal character escaping", Match(SymbolTrie::String, std::move(value), reader.SourceSite()));

                    ch = (char)((d0 <= '9' ? d0 - '0' : d0 - 'a') * 16 +
                                (d1 <= '9' ? d1 - '0' : d1 - 'a') );

                    value += ch;

                    break;

                case '\0':
                    throw LexerException("unterminated weak quoted string", Match(SymbolTrie::String, std::move(value), reader.SourceSite()));

                default:
                    throw LexerException("invalid character escaping", Match(SymbolTrie::String, std::move(value), reader.SourceSite()));
                }
            }
            else
            {
                switch (ch)
                {
                case '"': inQuote = false; break;
                case '\\': escaped = true; break;

                case '\0':
                    throw LexerException("unterminated weak quoted string", Match(SymbolTrie::String, std::move(value), reader.SourceSite()));

                default:
                    value += ch;
                    break;
                }
            }
        } while (inQuote);

        Assert(value.size());
        return true;
    }

    *psymbol = SymbolTrie::Invalid;
    Assert(value.empty());

    return false;
}
//----------------------------------------------------------------------------
static bool Lex_Identifier_(LookAheadReader& reader, const Symbol **psymbol, String& value) {
    Assert(psymbol);
    Assert(value.empty());

    char ch = reader.Peek(0);

    if (IsAlpha(ch) || ('_' == ch))
    {
        *psymbol = SymbolTrie::Identifier;
        return ReadCharset_(Identifier_, reader, value);
    }

    *psymbol = SymbolTrie::Invalid;
    Assert(value.empty());

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Lexer::Lexer(const StringSlice& input, const wchar_t *sourceFileName)
:   _sourceFileName(sourceFileName)
,   _reader(input, _sourceFileName.c_str())
,   _peeking(false) {
    _lexing.reserve(512);
}
//----------------------------------------------------------------------------
Lexer::~Lexer() {}
//----------------------------------------------------------------------------
const Match *Lexer::Peek() {
    if (!_peeking) {
        _peeking = true;
        NextMatch_(_peek);
    }
    return ((_peek.Valid()) ? &_peek : nullptr);
}
//----------------------------------------------------------------------------
bool Lexer::Read(Match& match) {
    return NextMatch_(match);
}
//----------------------------------------------------------------------------
bool Lexer::NextMatch_(Match& match) {
    if (_peeking) {
        _peeking = false;
        match = std::move(_peek);
        return (Symbol::Eof != match.Symbol()->Type());
    }

    _reader.EatWhiteSpaces();

    if ('\0' == _reader.Peek(0)) {
        match._symbol = SymbolTrie::Eof;
        match._value.clear();
        match._site = _reader.SourceSite();
        return false;
    }

    Lex_Comments_(_reader);

    match._site = _reader.SourceSite();

    const Symbol *psymbol = nullptr;
    Assert(_lexing.empty());

    // by priority order :
    if (Lex_Numeric_(_reader, &psymbol, _lexing) ||
        Lex_String_(_reader, &psymbol, _lexing) ||
        Lex_Symbol_(_reader, &psymbol) ||
        Lex_Identifier_(_reader, &psymbol, _lexing)
        ) {
        Assert(psymbol);
        Assert(Symbol::Eof != psymbol->Type());
        Assert(Symbol::Invalid != psymbol->Type());

        match._symbol = psymbol;
        match._value = _lexing;

        _lexing.clear();

        return true;
    }

    match._symbol = SymbolTrie::Invalid;
    match._value.clear();

    _lexing.clear();

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void LexerStartup::Start() {
    SymbolTrie::Create();
}
//----------------------------------------------------------------------------
void LexerStartup::Shutdown() {
    SymbolTrie::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
