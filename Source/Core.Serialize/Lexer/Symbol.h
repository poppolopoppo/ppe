#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/Hash.h"
#include "Core/IO/StringView.h"

#include <iosfwd>

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSymbol {
public:

    enum ETypeId : u64 {
        Invalid         = 0,

        Eof             = 1ull<< 0,

        Int             = 1ull<< 1,   // 42 or 033 or 0xFE
        Float           = 1ull<< 2,   // 0.5 or 1e5
        String          = 1ull<< 3,   // "dsf\"dsfs" or 'sdfdsf'
        Identifier      = 1ull<< 4,   // [_a-zA-Z][\w\d]*

        True            = 1ull<< 5,   // true
        False           = 1ull<< 6,   // false

        Nil             = 1ull<< 7,   // nil

        Is              = 1ull<< 8,   // is
        Extern          = 1ull<< 9,   // extern
        Export          = 1ull<<10,   // export

        LBrace          = 1ull<<11,   // {
        RBrace          = 1ull<<12,   // }

        LBracket        = 1ull<<13,   // [
        RBracket        = 1ull<<14,   // ]

        LParenthese     = 1ull<<15,   // (
        RParenthese     = 1ull<<16,   // )

        Comma           = 1ull<<17,   // ,
        Colon           = 1ull<<18,   // :
        SemiColon       = 1ull<<19,   // ;
        Dot             = 1ull<<20,   // .
        Dollar          = 1ull<<21,   // $
        Question        = 1ull<<22,   // ?

        Add             = 1ull<<23,   // +
        Sub             = 1ull<<24,   // -
        Mul             = 1ull<<25,   // *
        Div             = 1ull<<26,   // /

        Mod             = 1ull<<27,   // %
        Pow             = 1ull<<28,   // **

        Increment       = 1ull<<29,   // ++
        Decrement       = 1ull<<30,   // --

        LShift          = 1ull<<31,   // <<
        RShift          = 1ull<<32,   // >>

        And             = 1ull<<33,   // &
        Or              = 1ull<<34,   // |
        TNot             = 1ull<<35,   // !

        Xor             = 1ull<<36,   // ^
        Complement      = 1ull<<37,   // ~

        Assignment      = 1ull<<38,   // =

        Equals          = 1ull<<39,   // ==
        NotEquals       = 1ull<<40,   // !=

        Less            = 1ull<<41,   // <
        LessOrEqual     = 1ull<<42,   // <=

        Greater         = 1ull<<43,   // >
        GreaterOrEqual  = 1ull<<44,   // >=

        DotDot          = 1ull<<45,   // ..
        Sharp           = 1ull<<46,   // #

        Typename        = 1ull<<50,   // float2,byte4,etc...

        Prefix          = 1ull<<63,   // for FSymbols class
    };
    ENUM_FLAGS_FRIEND(ETypeId);

    FSymbol() : _type(Invalid), _ord(0) {}
    FSymbol(ETypeId type, const FStringView& cstr, u64 ord = 0) : _type(type), _cstr(cstr), _ord(ord) {}

    bool IsValid() const { return (Meta::popcnt64(u64(_type)) == 1); }
    bool IsPrefix() const { return (_type ^ Prefix); }

    ETypeId Type() const { return _type; }
    const FStringView& CStr() const { return _cstr; }
    u64 Ord() const { return _ord; }

private:
    ETypeId _type;
    FStringView _cstr;
    u64 _ord;
};
//----------------------------------------------------------------------------
inline bool operator ==(const FSymbol& lhs, const FSymbol& rhs) {
    return lhs.Type() == rhs.Type();
}
//----------------------------------------------------------------------------
inline bool operator !=(const FSymbol& lhs, const FSymbol& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FSymbol& symbol) {
    return Core::hash_value(u64(symbol.Type()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T >
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const FSymbol& symbol) {
    return oss << symbol.CStr();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
