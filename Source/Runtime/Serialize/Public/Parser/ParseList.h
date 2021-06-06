#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "Lexer/Match.h"
#include "Lexer/Symbol.h"

#include "Allocator/SlabHeap.h"

namespace PPE {
namespace Lexer {
    class FLexer;
}

namespace Parser {
struct FParseResult;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParseMatch : public Lexer::FMatchView {
public:
    FParseMatch(
        const Lexer::FSymbol* symbol,
        const FStringView& value,
        const Lexer::FSpan& site ) NOEXCEPT
    :   Lexer::FMatchView(symbol, value, site)
    {}

    using Lexer::FMatchView::Symbol;
    using Lexer::FMatchView::Value;
    using Lexer::FMatchView::Site;
    using Lexer::FMatchView::MakeView;
    using Lexer::FMatchView::Valid;

    TIntrusiveListNode<FParseMatch> Node{ nullptr, nullptr };
};
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FParseList {
public:
    FParseList() NOEXCEPT;
    ~FParseList();

    FParseList(FParseList&& rvalue) = delete;
    FParseList& operator =(FParseList&& rvalue) = delete;

    FParseList(const FParseList&) = delete;
    FParseList& operator =(const FParseList&) = delete;

    bool empty() const { return _list.empty(); }

    const Lexer::FSpan& Site() const { return _site; }

    const FParseMatch *Peek() const { return _curr; }
    Lexer::FSymbol::ETypeId PeekType() const { return (_curr ? _curr->Symbol()->Type() : Lexer::FSymbol::Eof); }

    bool Parse(Lexer::FLexer* lexer);

    void Reset() NOEXCEPT { Seek(_list.Head()); }
    void Seek(const FParseMatch* match) NOEXCEPT;

    template <Lexer::FSymbol::ETypeId _Symbol>
    bool Expect(const FParseMatch** m) {
        return ((*m = Read()) != nullptr && (*m)->Symbol()->Type() ^ _Symbol);
    }
    template <Lexer::FSymbol::ETypeId _Symbol>
    bool TryRead(const FParseMatch** m) {
        if (PeekType() ^ _Symbol) {
            *m = Read();
            return true;
        }
        return false;
    }

    NODISCARD const FParseMatch* Read();

    NORETURN void Error(const FParseResult& result) const;
    NORETURN void Error(const char* what, const Lexer::FSpan& site) const;

    void Clear();

private:
    const FParseMatch* _curr;
    Lexer::FSpan _site;

    INTRUSIVELIST(&FParseMatch::Node) _list;
    SLABHEAP(Lexer) _heap;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
