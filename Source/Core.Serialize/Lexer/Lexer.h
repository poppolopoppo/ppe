#pragma once

#include "Core/Core.h"

#include "Core.Serialize/Lexer/LookAheadReader.h"
#include "Core.Serialize/Lexer/Match.h"
#include "Core/IO/String.h"

#include <stdexcept>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LexerException : public std::logic_error {
public:
    LexerException(const char *what, Match&& match)
        :   std::logic_error(what)
        ,   _match(std::move(match)) {}

    virtual ~LexerException() {}

    const Core::Lexer::Match& Match() const { return _match; }

private:
    Core::Lexer::Match _match;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Lexer {
public:
    Lexer(const StringSlice& input, const char *sourceFileName);
    ~Lexer();

    const Match *Peek();
    bool Read(Match& match);

    const String& SourceFileName() { return _sourceFileName; }

private:
    bool NextMatch_(Match& match);

    String _sourceFileName;
    LookAheadReader _reader;

    String _lexing;
    Match _peek;
    bool _peeking;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct LexerStartup {
    static void Start();
    static void Shutdown();

    LexerStartup() { Start(); }
    ~LexerStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
