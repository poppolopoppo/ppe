#include "stdafx.h"

#include "Parser/ParseList.h"

#include "Parser/Parser.h"
#include "Parser/ParseResult.h"

#include "Lexer/Lexer.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FParseList::FParseList() NOEXCEPT
:   _curr(nullptr)
,   _site(Lexer::FLocation::None(), 0)
{}
//----------------------------------------------------------------------------
FParseList::~FParseList() {
    Clear();
}
//----------------------------------------------------------------------------
bool FParseList::Parse(Lexer::FLexer* lexer /* = nullptr */) {
    Assert(lexer);

    Clear();

    // replicate lexer matches on the heap to get nice batching of allocations
    {
        Lexer::FMatch m;
        while (lexer->Read(m)) {
            char* cpy = nullptr;
            const size_t s = m.Value().size();

            if (s) { // copy the string on the heap IFN
                cpy = static_cast<char*>(_heap.Allocate(m.Value().SizeInBytes()));
                FPlatformMemory::Memcpy(cpy, m.Value().data(), m.Value().SizeInBytes());
            }

            // also construct the match on the heap
            const FStringView value(cpy, s);
            _list.PushTail(new (_heap) FParseMatch{ m.Symbol(), value, m.Site() });

        }
    }

    if (_list.Head()) {
        Seek(_list.Head());
        return true;
    }
    else {
        _site.Filename = lexer->SourceFileName();
        return false;
    }
}
//----------------------------------------------------------------------------
void FParseList::Seek(const FParseMatch* match) NOEXCEPT {
    if (nullptr == match) {
        _curr = nullptr;

        // jump to the end of the parse list
        if (const FParseMatch * tail = _list.Tail()) {
            _site = tail->Site();
            _site.Offset += _site.Length;
            _site.Column += _site.Length;
            _site.Length = 0;
        }
    }
    else {
        Assert(match);
        Assert_NoAssume(_list.Head());

        _curr = match;
        _site = match->Site();
    }
}
//----------------------------------------------------------------------------
const FParseMatch* FParseList::Read() {
    if (nullptr == _curr)
        return nullptr;

    Assert_NoAssume(_list.Head());

    const FParseMatch* const read = _curr;
    Seek(_curr->Node.Next);

    return read;
}
//----------------------------------------------------------------------------
void FParseList::Clear() {
    _curr = nullptr;
    _list.Clear();
    _heap.ReleaseAll(); // FParseMatch is trivially destructible
    _site = { Lexer::FLocation::None(), 0 };
}
//----------------------------------------------------------------------------
void NORETURN FParseList::Error(const FParseResult& result) const {
    Error(result.Error, result.Site);
}
//----------------------------------------------------------------------------
void NORETURN FParseList::Error(const char* what, const Lexer::FSpan& site) const {
    PPE_THROW_IT(FParserException(what, site, nullptr));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
