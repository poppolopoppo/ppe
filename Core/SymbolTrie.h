#pragma once

#include "Core.h"
#include "Allocation.h"
#include "Pair.h"
#include "String.h"
#include "Trie.h"

#include "Symbol.h"

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SymbolTrie : public Core::Trie<
    char,
    Symbol,
    CharEqualTo<char, CaseSensitive::False>,
    ALLOCATOR(Lexer, Pair<char COMMA Symbol>)
> {
public:
    typedef Core::Trie<
        char,
        Symbol,
        CharEqualTo<char, CaseSensitive::False>,
        ALLOCATOR(Lexer, Pair<char COMMA Symbol>)
    >   parent_type;

    SymbolTrie();
    ~SymbolTrie();

    static void Create();
    static void Destroy();

    static const SymbolTrie& Instance();
    static bool HasInstance();

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

private:
    const Symbol *Insert_(Symbol::TypeId type, const char *cstr, size_t size);

    template <size_t _Dim>
    const Symbol *Insert_(Symbol::TypeId type, const char(&staticArray)[_Dim]) {
        static_assert(_Dim, "invalid string");
        Assert(!staticArray[_Dim - 1]);
        return Insert_(type, staticArray, _Dim - 1 /* assume null terminated string */);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
