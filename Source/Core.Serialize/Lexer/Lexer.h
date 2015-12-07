#pragma once

#include "Core/Core.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Lexer/LookAheadReader.h"
#include "Core.Serialize/Lexer/Match.h"

#include "Core/IO/String.h"

#include <stdexcept>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LexerException : public Core::Serialize::SerializeException {
public:
    typedef Core::Serialize::SerializeException parent_type;

    LexerException(const char *what, Match&& match)
        :   parent_type(what)
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
    Lexer(const StringSlice& input, const wchar_t *sourceFileName);
    ~Lexer();

    const Match *Peek();
    bool Read(Match& match);

    const WString& SourceFileName() { return _sourceFileName; }

private:
    bool NextMatch_(Match& match);

    WString _sourceFileName;
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
