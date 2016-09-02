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
class Lexer;
//----------------------------------------------------------------------------
class Match {
public:
    friend class Lexer;
    typedef Core::Lexer::Symbol symbol_type;

    Match();
    ~Match();

    Match(const symbol_type *symbol, String&& rvalue, const Location& site, size_t offset);
    Match(const symbol_type *symbol, const String& value, const Location& site, size_t offset);

    const symbol_type *Symbol() const { return _symbol; }

    String& Value() { return _value; }
    const String& Value() const { return _value; }

    const Location& Site() const { return _site; }

    size_t Offset() const { return _offset; }

    StringView MakeView() const { return MakeStringView(_value); }

    bool Valid() const { return symbol_type::Invalid != _symbol->Type(); }

private:
    const symbol_type *_symbol;
    String _value;
    Location _site;
    size_t _offset;
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
