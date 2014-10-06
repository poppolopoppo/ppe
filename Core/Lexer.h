#pragma once

#include "Core.h"

#include "LookAheadReader.h"
#include "String.h"
#include "Match.h"

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LexerException : public std::exception {
public:
    LexerException(const char *what, Match&& match)
        : std::exception(what), _match(std::move(match)) {}

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
    explicit Lexer(const StringSlice& input);
    Lexer(IVirtualFileSystemIStream *stream, const char *sourceFileName);
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
