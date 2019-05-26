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
template <typename T>
class TBasicMatch {
public:
    typedef T value_type;
    typedef PPE::Lexer::FSymbol symbol_type;

    TBasicMatch() NOEXCEPT;
    ~TBasicMatch();

    TBasicMatch(const symbol_type* symbol, value_type&& rvalue, const FSpan& site) NOEXCEPT
        : _symbol(symbol), _value(std::move(rvalue)), _site(site)
    {}
    TBasicMatch(const symbol_type* symbol, const value_type& value, const FSpan& site) NOEXCEPT
        : TBasicMatch(symbol, value_type(value), site)
    {}
    TBasicMatch(const symbol_type* symbol, value_type&& rvalue, const FLocation& start, const FLocation& stop) NOEXCEPT
        : TBasicMatch(symbol, std::move(rvalue), FSpan::FromSite(start, stop))
    {}
    TBasicMatch(const symbol_type* symbol, const value_type& value, const FLocation& start, const FLocation& stop) NOEXCEPT
        : TBasicMatch(symbol, value_type(value), start, stop)
    {}

    const symbol_type *Symbol() const { return _symbol; }
    value_type& Value() { return _value; }
    const value_type& Value() const { return _value; }
    const FSpan& Site() const { return _site; }

    FStringView MakeView() const { return MakeStringView(_value); }

    bool Valid() const { return symbol_type::Invalid != _symbol->Type(); }

private:
    const symbol_type* _symbol;
    value_type _value;
    FSpan _site;
};
//----------------------------------------------------------------------------
using FMatch = TBasicMatch<FString>;
using FMatchView = TBasicMatch<FStringView>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE

EXTERN_TEMPLATE_CLASS_DECL(PPE_SERIALIZE_API) PPE::Lexer::TBasicMatch<PPE::FString>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_SERIALIZE_API) PPE::Lexer::TBasicMatch<PPE::FStringView>;

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
