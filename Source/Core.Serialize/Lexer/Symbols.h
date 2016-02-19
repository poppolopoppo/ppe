#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/StringHashMap.h"
#include "Core/Meta/Singleton.h"

namespace Core {
namespace Lexer {
class Symbol;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Symbols : Meta::Singleton<Symbols>, Meta::ThreadResource {
public:
    STATIC_CONST_INTEGRAL(size_t, MaxLength, 32);
    typedef STRINGSLICE_HASHMAP(Lexer, Symbol, CaseSensitive::True) hashmap_type;
private:
    typedef Meta::Singleton<Symbols> singleton_type;
    friend class Meta::SingletonHelpers<Symbols, singleton_type>;

    hashmap_type _symbols;

    Symbols();
public:
    ~Symbols();

    using singleton_type::HasInstance;
    using singleton_type::Destroy;

    static void Create() { singleton_type::Create(); }
    static const Symbols& Instance() { return singleton_type::Instance(); }

    const hashmap_type& All() const { THIS_THREADRESOURCE_CHECKACCESS(); return _symbols; }

    bool IsPrefix(const Symbol** psymbol, const StringSlice& cstr) const;

public:
    static const Symbol *Invalid;

    static const Symbol *Eof;

    static const Symbol *Int;
    static const Symbol *Float;
    static const Symbol *String;
    static const Symbol *Identifier;

    static const Symbol *True;
    static const Symbol *False;
    static const Symbol *Nil;
    static const Symbol *Is;
    static const Symbol *Extern;
    static const Symbol *Export;
    static const Symbol *LBrace;
    static const Symbol *RBrace;
    static const Symbol *LBracket;
    static const Symbol *RBracket;
    static const Symbol *LParenthese;
    static const Symbol *RParenthese;
    static const Symbol *Comma;
    static const Symbol *Colon;
    static const Symbol *SemiColon;
    static const Symbol *Dot;
    static const Symbol *Dollar;
    static const Symbol *Question;
    static const Symbol *Add;
    static const Symbol *Sub;
    static const Symbol *Mul;
    static const Symbol *Div;
    static const Symbol *Mod;
    static const Symbol *Pow;
    static const Symbol *Increment;
    static const Symbol *Decrement;
    static const Symbol *LShift;
    static const Symbol *RShift;
    static const Symbol *And;
    static const Symbol *Or;
    static const Symbol *Not;
    static const Symbol *Xor;
    static const Symbol *Complement;
    static const Symbol *Assignment;
    static const Symbol *Equals;
    static const Symbol *NotEquals;
    static const Symbol *Less;
    static const Symbol *LessOrEqual;
    static const Symbol *Greater;
    static const Symbol *GreaterOrEqual;
    static const Symbol *DotDot;

    static const Symbol *Typename;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
