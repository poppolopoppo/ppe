#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core.Serialize/Lexer/Match.h"
#include "Core.Serialize/Lexer/Symbol.h"

#include "Core/Container/Vector.h"

namespace Core {
namespace Lexer {
    class FLexer;
}

namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParseList {
public:
    FParseList();
    ~FParseList();

    FParseList(FParseList&& rvalue);
    FParseList& operator =(FParseList&& rvalue);

    FParseList(const FParseList&) = delete;
    FParseList& operator =(const FParseList&) = delete;

    bool empty() const { return _matches.empty(); }

    const Lexer::FLocation& Site() const { return _site; }

    const Lexer::FMatch *Peek() const { return _current; }
    Lexer::FSymbol::ETypeId PeekType() const { return (_current) ? _current->Symbol()->Type() : Lexer::FSymbol::Eof; }

    const VECTOR(Parser, Lexer::FMatch)& Matches() const { return _matches; }

    bool Parse(Lexer::FLexer* lexer);
    void Reset();
    void Seek(const Lexer::FMatch *match);
    const Lexer::FMatch *Read();

private:
    Lexer::FLocation _site;
    const Lexer::FMatch *_current;
    VECTOR(Parser, Lexer::FMatch) _matches;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
