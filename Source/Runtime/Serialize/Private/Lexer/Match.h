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
    friend class FLexer;
    typedef Core::Lexer::FSymbol symbol_type;

    FMatch();
    ~FMatch();

    FMatch(const symbol_type *symbol, FString&& rvalue, const FLocation& site, size_t offset);
    FMatch(const symbol_type *symbol, const FString& value, const FLocation& site, size_t offset);

    const symbol_type *Symbol() const { return _symbol; }

    FString& Value() { return _value; }
    const FString& Value() const { return _value; }

    const FLocation& Site() const { return _site; }

    size_t Offset() const { return _offset; }

    FStringView MakeView() const { return MakeStringView(_value); }

    bool Valid() const { return symbol_type::Invalid != _symbol->Type(); }

private:
    const symbol_type *_symbol;
    FString _value;
    FLocation _site;
    size_t _offset;
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
