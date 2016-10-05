#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core.Serialize/Lexer/Match.h"
#include "Core.Serialize/Lexer/Symbol.h"

#include "Core/Container/Vector.h"

namespace Core {
namespace FLexer {
    class FLexer;
}

namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParseList {
public:
    FParseList(FLexer::FLexer *lexer);
    ~FParseList();

    FParseList(FParseList&& rvalue);
    FParseList& operator =(FParseList&& rvalue);

    FParseList(const FParseList&) = delete;
    FParseList& operator =(const FParseList&) = delete;

    bool empty() const { return _matches.empty(); }

    const FLexer::FLocation& Site() const { return _site; }

    const FLexer::FMatch *Peek() const { return _current; }
    FLexer::FSymbol::ETypeId PeekType() const { return (_current) ? _current->Symbol()->Type() : FLexer::FSymbol::Eof; }

    const VECTOR(Parser, FLexer::FMatch)& Matches() const { return _matches; }

    void Reset();
    void Seek(const FLexer::FMatch *match);
    const FLexer::FMatch *Read();

private:
    FLexer::FLocation _site;
    const FLexer::FMatch *_current;
    VECTOR(Parser, FLexer::FMatch) _matches;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
