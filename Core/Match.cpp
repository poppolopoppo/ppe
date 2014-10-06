#include "stdafx.h"

#include "Match.h"

#include "SymbolTrie.h"

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Match::Match()
:   _symbol(SymbolTrie::Invalid), _site(nullptr, 0, 0) {}
//----------------------------------------------------------------------------
Match::~Match() {}
//----------------------------------------------------------------------------
Match::Match(const symbol_type *symbol, String&& rvalue, const Location& site)
:   _symbol(symbol), _value(std::move(rvalue)), _site(site) {
    Assert(_symbol);
}
//----------------------------------------------------------------------------
Match::Match(const symbol_type *symbol, const String& value, const Location& site)
:   _symbol(symbol), _value(value), _site(site) {
    Assert(_symbol);
}
//----------------------------------------------------------------------------
Match::Match(Match&& rvalue)
:   _symbol(std::move(rvalue._symbol))
,   _value(std::move(rvalue._value))
,   _site(std::move(rvalue._site)) {
}
//----------------------------------------------------------------------------
Match& Match::operator =(Match&& rvalue) {
    _symbol = std::move(rvalue._symbol);
    _value = std::move(rvalue._value);
    _site = std::move(rvalue._site);
    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
