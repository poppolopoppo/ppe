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

    TBasicMatch(const TBasicMatch&) = default;
    TBasicMatch& operator =(const TBasicMatch&) = default;

    TBasicMatch(TBasicMatch&& rvalue) NOEXCEPT;
    TBasicMatch& operator =(TBasicMatch&& rvalue) NOEXCEPT;

    TBasicMatch(const symbol_type* symbol, value_type&& rvalue, const FSpan& site) NOEXCEPT;
    TBasicMatch(const symbol_type* symbol, const value_type& value, const FSpan& site) NOEXCEPT;
    TBasicMatch(const symbol_type* symbol, value_type&& rvalue, const FLocation& start, const FLocation& stop) NOEXCEPT;
    TBasicMatch(const symbol_type* symbol, const value_type& value, const FLocation& start, const FLocation& stop) NOEXCEPT;

    const symbol_type *Symbol() const;
    value_type& Value();
    const value_type& Value() const;
    const FSpan& Site() const;

    FStringView MakeView() const;

    bool Valid() const;

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

#ifndef EXPORT_PPE_RUNTIME_SERIALIZE_BASICMATCH
EXTERN_TEMPLATE_CLASS_DECL(PPE_SERIALIZE_API) PPE::Lexer::TBasicMatch<PPE::FString>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_SERIALIZE_API) PPE::Lexer::TBasicMatch<PPE::FStringView>;
#endif

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
