#include "stdafx.h"

#include "Parser/ParseList.h"

#include "Parser/Parser.h"
#include "Parser/ParseResult.h"

#include "Diagnostic/Logger.h"
#include "Lexer/Lexer.h"

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FParseList::FParseList()
:   _site(Lexer::FLocation::None(), 0)
,   _current(nullptr)
{}
//----------------------------------------------------------------------------
FParseList::~FParseList() {}
//----------------------------------------------------------------------------
FParseList::FParseList(FParseList&& rvalue)
:   _site(Lexer::FLocation::None(), 0)
,   _current(nullptr)
,   _matches(std::move(rvalue._matches)) {
    std::swap(rvalue._current, _current);
    std::swap(rvalue._site, _site);
}
//----------------------------------------------------------------------------
FParseList& FParseList::operator =(FParseList&& rvalue) {
    _current = nullptr;
    _matches = std::move(rvalue._matches);
    std::swap(rvalue._current, _current);
    return *this;
}
//----------------------------------------------------------------------------
bool FParseList::Parse(Lexer::FLexer* lexer /* = nullptr */) {
    Assert(lexer);

    _matches.clear();

    Lexer::FMatch match;
    while (lexer->Read(match)) {
        //LOG(Info, L"[Parser] match : <{0}> = '{1}'", match.Symbol()->CStr().Pointer(), match.Value().c_str());
        _matches.emplace_back(std::move(match));
    }

    if (_matches.size())
        _site = _matches.front().Site();
    else
        _site.Filename = lexer->SourceFileName();

    Reset();

    return (not _matches.empty());
}
//----------------------------------------------------------------------------
void FParseList::Reset() {
    if (_matches.size())
        _current = &_matches.front();
    else
        _current = nullptr;
}
//----------------------------------------------------------------------------
void FParseList::Seek(const Lexer::FMatch *match) {
    if (nullptr == match) {
        Assert(nullptr == _current);
    }
    else {
        Assert(match);
        Assert(_matches.size());
        Assert(&_matches.front() <= match && &_matches.back() >= match);

        _current = match;
    }
}
//----------------------------------------------------------------------------
const Lexer::FMatch *FParseList::Read() {
    if (nullptr == _current)
        return nullptr;

    Assert(_matches.size());
    Assert(&_matches.front() <= _current && &_matches.back() >= _current);

    const Lexer::FMatch *read = _current;

    if (&_matches.back() == _current)
        _current = nullptr;
    else
        ++_current;

    if (_current)
        _site = _current->Site();

    return read;
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
