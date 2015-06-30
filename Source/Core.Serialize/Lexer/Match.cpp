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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
