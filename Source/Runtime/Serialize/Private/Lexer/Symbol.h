#pragma once

#include "Serialize.h"

#include "Container/Hash.h"
#include "HAL/PlatformMaths.h"
#include "IO/StringView.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSymbol {
public:

    enum ETypeId : u64 {
        Invalid         = 0,

        Eof             = 1ull<< 0,

        Integer         = 1ull<< 1,   // 42 or 033 or 0xFE
        Unsigned        = 1ull<< 2,   // 42u or 033U or 0xFEu

        Float           = 1ull<< 3,   // 0.5 or 1e5
        String          = 1ull<< 4,   // "dsf\"dsfs" or 'sdfdsf'
        Identifier      = 1ull<< 5,   // [_a-zA-Z][\w\d]*

        True            = 1ull<< 6,   // true
        False           = 1ull<< 7,   // false

        Null            = 1ull<< 8,   // null

        Is              = 1ull<< 9,   // is
        Extern          = 1ull<<10,   // extern
        Export          = 1ull<<11,   // export

        LBrace          = 1ull<<12,   // {
        RBrace          = 1ull<<13,   // }

        LBracket        = 1ull<<14,   // [
        RBracket        = 1ull<<15,   // ]

        LParenthese     = 1ull<<16,   // (
        RParenthese     = 1ull<<17,   // )

        Comma           = 1ull<<18,   // ,
        Colon           = 1ull<<19,   // :
        SemiColon       = 1ull<<20,   // ;
        Dot             = 1ull<<21,   // .
        Dollar          = 1ull<<22,   // $
        Question        = 1ull<<23,   // ?

        Add             = 1ull<<24,   // +
        Sub             = 1ull<<25,   // -
        Mul             = 1ull<<26,   // *
        Div             = 1ull<<27,   // /

        Mod             = 1ull<<28,   // %
        Pow             = 1ull<<29,   // **

        Increment       = 1ull<<30,   // ++
        Decrement       = 1ull<<31,   // --

        LShift          = 1ull<<32,   // <<
        RShift          = 1ull<<33,   // >>

        And             = 1ull<<34,   // &
        Or              = 1ull<<35,   // |
        Not             = 1ull<<36,   // !

        Xor             = 1ull<<37,   // ^
        Complement      = 1ull<<38,   // ~

        Assignment      = 1ull<<39,   // =

        Equals          = 1ull<<40,   // ==
        NotEquals       = 1ull<<41,   // !=

        Less            = 1ull<<42,   // <
        LessOrEqual     = 1ull<<43,   // <=

        Greater         = 1ull<<44,   // >
        GreaterOrEqual  = 1ull<<45,   // >=

        DotDot          = 1ull<<46,   // ..
        Sharp           = 1ull<<47,   // #

        Typename        = 1ull<<50,   // Any,BinaryData,Name...

        Prefix          = 1ull<<63,   // for FSymbols class
    };
    ENUM_FLAGS_FRIEND(ETypeId);

    FSymbol() : _type(Invalid), _ord(0) {}
    FSymbol(ETypeId type, const FStringView& cstr, u64 ord = 0) : _type(type), _cstr(cstr), _ord(ord) {}

    bool IsValid() const { return (FPlatformMaths::popcnt64(u64(_type)) == 1); }
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
    return not operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FSymbol& symbol) {
    return Core::hash_value(u64(symbol.Type()) );
}
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
TBasicTextWriter<_Char>& operator <<(
    TBasicTextWriter<_Char>& oss,
    const Lexer::FSymbol& symbol) {
    return oss << symbol.CStr();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
