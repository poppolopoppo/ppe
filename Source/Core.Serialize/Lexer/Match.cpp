#include "stdafx.h"

#include "Match.h"

#include "Symbols.h"

#include "Core/IO/StringView.h"

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMatch::FMatch()
:   _symbol(FSymbols::Invalid)
,   _site(FWStringView(), 0, 0)
,   _offset(size_t(-1)) {}
//----------------------------------------------------------------------------
FMatch::~FMatch() {}
//----------------------------------------------------------------------------
FMatch::FMatch(const symbol_type *symbol, FString&& rvalue, const FLocation& site, size_t offset)
:   _symbol(symbol)
,   _value(std::move(rvalue))
,   _site(site)
,   _offset(offset) {
    Assert(_symbol);
}
//----------------------------------------------------------------------------
FMatch::FMatch(const symbol_type *symbol, const FString& value, const FLocation& site, size_t offset)
:   _symbol(symbol)
,   _value(value)
,   _site(site)
,   _offset(offset) {
    Assert(_symbol);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
