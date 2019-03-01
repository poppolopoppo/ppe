#include "stdafx.h"

#include "Lexer/Match.h"

#include "Lexer/Symbols.h"

#include "IO/StringView.h"

namespace PPE {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMatch::FMatch()
:   _symbol(FSymbols::Invalid) {
    Assert(_symbol);
}
//----------------------------------------------------------------------------
FMatch::~FMatch() {}
//----------------------------------------------------------------------------
FMatch::FMatch(const symbol_type *symbol, FString&& rvalue, const FLocation& start, const FLocation& stop)
:   _symbol(symbol)
,   _value(std::move(rvalue))
,   _site(FSpan::FromSite(start, stop)) {
    Assert(_symbol);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE
