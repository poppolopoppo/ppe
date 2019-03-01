#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "Lexer/Symbol.h"
#include "IO/String.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLexer;
//----------------------------------------------------------------------------
class FMatch {
public:
    typedef PPE::Lexer::FSymbol symbol_type;

    FMatch();
    ~FMatch();

    FMatch(const symbol_type *symbol, FString&& rvalue, const FLocation& start, const FLocation& stop);
    FMatch(const symbol_type *symbol, const FString& value, const FLocation& start, const FLocation& stop)
        : FMatch(symbol, FString(value), start, stop)
    {}

    const symbol_type *Symbol() const { return _symbol; }
    FString& Value() { return _value; }
    const FString& Value() const { return _value; }
    const FSpan& Site() const { return _site; }

    FStringView MakeView() const { return MakeStringView(_value); }

    bool Valid() const { return symbol_type::Invalid != _symbol->Type(); }

private:
    const symbol_type *_symbol;
    FString _value;
    FSpan _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Lexer::FMatch& match) {
    if (match.Symbol())
        return oss << "<" << match.Symbol()->CStr() << "> = \"" << match.Value() << "\"";
    else
        return oss << "nil";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
