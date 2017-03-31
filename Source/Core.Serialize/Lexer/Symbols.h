#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/StringHashMap.h"
#include "Core/Meta/Singleton.h"

namespace Core {
namespace Lexer {
class FSymbol;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSymbols : Meta::TSingleton<FSymbols>, Meta::FThreadResource {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxLength, 32);
    typedef STRINGVIEW_HASHMAP(Lexer, FSymbol, ECase::Sensitive) hashmap_type;
private:
    friend class Meta::TSingleton<FSymbols>;
    typedef Meta::TSingleton<FSymbols> singleton_type;

    hashmap_type _symbols;

    FSymbols();
public:
    ~FSymbols();

    using singleton_type::HasInstance;
    using singleton_type::Destroy;

    static void Create() { singleton_type::Create(); }
    static const FSymbols& Instance() { return singleton_type::Instance(); }

    const hashmap_type& All() const { THIS_THREADRESOURCE_CHECKACCESS(); return _symbols; }

    bool IsPrefix(const FSymbol** psymbol, const FStringView& cstr) const;

public:
    static const FSymbol *Invalid;

    static const FSymbol *Eof;

    static const FSymbol *Int;
    static const FSymbol *Float;
    static const FSymbol *String;
    static const FSymbol *Identifier;

    static const FSymbol *True;
    static const FSymbol *False;
    static const FSymbol *Nil;
    static const FSymbol *Is;
    static const FSymbol *Extern;
    static const FSymbol *Export;
    static const FSymbol *LBrace;
    static const FSymbol *RBrace;
    static const FSymbol *LBracket;
    static const FSymbol *RBracket;
    static const FSymbol *LParenthese;
    static const FSymbol *RParenthese;
    static const FSymbol *Comma;
    static const FSymbol *Colon;
    static const FSymbol *SemiColon;
    static const FSymbol *Dot;
    static const FSymbol *Dollar;
    static const FSymbol *Question;
    static const FSymbol *Add;
    static const FSymbol *Sub;
    static const FSymbol *Mul;
    static const FSymbol *Div;
    static const FSymbol *Mod;
    static const FSymbol *Pow;
    static const FSymbol *Increment;
    static const FSymbol *Decrement;
    static const FSymbol *LShift;
    static const FSymbol *RShift;
    static const FSymbol *And;
    static const FSymbol *Or;
    static const FSymbol *TNot;
    static const FSymbol *Xor;
    static const FSymbol *Complement;
    static const FSymbol *Assignment;
    static const FSymbol *Equals;
    static const FSymbol *NotEquals;
    static const FSymbol *Less;
    static const FSymbol *LessOrEqual;
    static const FSymbol *Greater;
    static const FSymbol *GreaterOrEqual;
    static const FSymbol *DotDot;
    static const FSymbol *Sharp;

    static const FSymbol *Typename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
