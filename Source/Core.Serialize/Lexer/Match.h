#pragma once

#include "Core/Core.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core.Serialize/Lexer/Symbol.h"
#include "Core/IO/String.h"

#include <iosfwd>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Lexer;
//----------------------------------------------------------------------------
class Match {
public:
    friend class Lexer;
    typedef Core::Lexer::Symbol symbol_type;

    Match();
    ~Match();

    Match(const symbol_type *symbol, String&& rvalue, const Location& site);
    Match(const symbol_type *symbol, const String& value, const Location& site);

    Match(Match&& rvalue);
    Match& operator =(Match&& rvalue);

    const symbol_type *Symbol() const { return _symbol; }
    const String& Value() const { return _value; }
    const Location& Site() const { return _site; }

    bool Valid() const { return symbol_type::Invalid != _symbol->Type(); }

private:
    const symbol_type *_symbol;
    String _value;
    Location _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Match& match) {
    if (match.Symbol()) {
        return oss << "<" << match.Symbol()->CStr() << "> = \"" << match.Value() << "\"";
    }
    else {
        return oss << "nil";
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
