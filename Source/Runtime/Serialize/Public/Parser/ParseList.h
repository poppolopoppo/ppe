#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "Lexer/Match.h"
#include "Lexer/Symbol.h"

#include "Container/Vector.h"

namespace PPE {
namespace Lexer {
    class FLexer;
}

namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FParseList {
public:
    FParseList();
    ~FParseList();

    FParseList(FParseList&& rvalue);
    FParseList& operator =(FParseList&& rvalue);

    FParseList(const FParseList&) = delete;
    FParseList& operator =(const FParseList&) = delete;

    bool empty() const { return _matches.empty(); }

    const Lexer::FSpan& Site() const { return _site; }

    const Lexer::FMatch *Peek() const { return _current; }
    Lexer::FSymbol::ETypeId PeekType() const { return (_current) ? _current->Symbol()->Type() : Lexer::FSymbol::Eof; }

    const VECTOR(Parser, Lexer::FMatch)& Matches() const { return _matches; }

    bool Parse(Lexer::FLexer* lexer);
    void Reset();
    void Seek(const Lexer::FMatch *match);
    const Lexer::FMatch *Read();

private:
    Lexer::FSpan _site;
    const Lexer::FMatch *_current;
    VECTOR(Parser, Lexer::FMatch) _matches;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
