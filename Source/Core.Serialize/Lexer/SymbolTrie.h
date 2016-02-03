#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/BurstTrie.h"
#include "Core/IO/String.h"
#include "Core/Meta/Singleton.h"

#include "Core.Serialize/Lexer/Symbol.h"

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef STRINGTRIE_MAP(Lexer, Symbol, CaseSensitive::False, 32) SymbolMap;
class SymbolRoot : Meta::ThreadResource {
public:
    SymbolMap& Map() { THIS_THREADRESOURCE_CHECKACCESS(); return _map; }
private:
    SymbolMap _map;
};
//----------------------------------------------------------------------------
class SymbolTrie : Meta::Singleton<SymbolRoot, SymbolTrie> {
public:
    typedef Meta::Singleton<SymbolRoot, SymbolTrie> singleton_type;
    typedef SymbolMap::query_t query_t;

    SymbolTrie() = delete;
    ~SymbolTrie() = delete;

    static void Create();
    static void Destroy();

    static query_t IsPrefix(const StringSlice& cstr, const query_t& hint);
    static query_t IsPrefix(char ch, const query_t& hint) { return IsPrefix(StringSlice(&ch, 1), hint); }

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
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
