#pragma once

#include "Serialize.h"

#include "Allocator/Allocation.h"
#include "Container/StringHashMap.h"
#include "Meta/Singleton.h"

namespace PPE {
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

#ifdef WITH_PPE_ASSERT
    using singleton_type::HasInstance;
#endif
    using singleton_type::Destroy;

    static void Create() { singleton_type::Create(); }
    static const FSymbols& Get() { return singleton_type::Get(); }

    const hashmap_type& All() const { THIS_THREADRESOURCE_CHECKACCESS(); return _symbols; }

    bool IsPrefix(const FSymbol** psymbol, const FStringView& cstr) const;

public:
    static const FSymbol *Invalid;

    static const FSymbol *Eof;

    static const FSymbol *Integer;
    static const FSymbol *Unsigned;
    static const FSymbol *Float;
    static const FSymbol *String;
    static const FSymbol *Identifier;

    static const FSymbol *True;
    static const FSymbol *False;
    static const FSymbol *Null;
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
    static const FSymbol *Not;
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
} //!namespace PPE
