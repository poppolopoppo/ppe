#include "stdafx.h"

#include "ParseList.h"

#include "Core/Diagnostic/Logger.h"
#include "Lexer/Lexer.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ParseList::ParseList(Lexer::Lexer *lexer)
:   _current(nullptr), _site(Lexer::Location::None()) {
    Assert(lexer);

    Lexer::Match match;
    while (lexer->Read(match)) {
        //LOG(Information, L"[Parser] match : <{0}> = '{1}'", match.Symbol()->CStr().Pointer(), match.Value().c_str());
        _matches.emplace_back(std::move(match));
    }

    if (_matches.size())
        _site = _matches.front().Site();
    else
        _site.FileName = lexer->SourceFileName().c_str();

    Reset();
}
//----------------------------------------------------------------------------
ParseList::~ParseList() {}
//----------------------------------------------------------------------------
ParseList::ParseList(ParseList&& rvalue)
:   _current(nullptr), _site(Lexer::Location::None()), _matches(std::move(rvalue._matches)) {
    std::swap(rvalue._current, _current);
    std::swap(rvalue._site, _site);
}
//----------------------------------------------------------------------------
ParseList& ParseList::operator =(ParseList&& rvalue) {
    _current = nullptr;
    _matches = std::move(rvalue._matches);
    std::swap(rvalue._current, _current);
    return *this;
}
//----------------------------------------------------------------------------
void ParseList::Reset() {
    if (_matches.size())
        _current = &_matches.front();
    else
        _current = nullptr;
}
//----------------------------------------------------------------------------
void ParseList::Seek(const Lexer::Match *match) {
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
const Lexer::Match *ParseList::Read() {
    if (nullptr == _current)
        return nullptr;

    Assert(_matches.size());
    Assert(&_matches.front() <= _current && &_matches.back() >= _current);

    const Lexer::Match *read = _current;

    if (&_matches.back() == _current)
        _current = nullptr;
    else
        ++_current;

    if (_current)
        _site = _current->Site();

    return read;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
