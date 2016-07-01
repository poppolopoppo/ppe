#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core.Serialize/Lexer/Match.h"
#include "Core.Serialize/Lexer/Symbol.h"

#include "Core/Container/Vector.h"

namespace Core {
namespace Lexer {
    class Lexer;
}

namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParseList {
public:
    ParseList(Lexer::Lexer *lexer);
    ~ParseList();

    ParseList(ParseList&& rvalue);
    ParseList& operator =(ParseList&& rvalue);

    ParseList(const ParseList&) = delete;
    ParseList& operator =(const ParseList&) = delete;

    bool empty() const { return _matches.empty(); }

    const Lexer::Location& Site() const { return _site; }

    const Lexer::Match *Peek() const { return _current; }
    Lexer::Symbol::TypeId PeekType() const { return (_current) ? _current->Symbol()->Type() : Lexer::Symbol::Eof; }

    const VECTOR(Parser, Lexer::Match)& Matches() const { return _matches; }

    void Reset();
    void Seek(const Lexer::Match *match);
    const Lexer::Match *Read();

private:
    Lexer::Location _site;
    const Lexer::Match *_current;
    VECTOR(Parser, Lexer::Match) _matches;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
