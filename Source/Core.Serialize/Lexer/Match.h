#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core.Serialize/Lexer/Symbol.h"
#include "Core/IO/String.h"

#include <iosfwd>

namespace Core {
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
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Lexer::FMatch& match) {
    if (match.Symbol())
        return oss << "<" << match.Symbol()->CStr() << "> = \"" << match.Value() << "\"";
    else
        return oss << "nil";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
