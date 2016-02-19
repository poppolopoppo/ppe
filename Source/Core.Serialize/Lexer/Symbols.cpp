#include "stdafx.h"

#include "Symbols.h"

#include "Symbol.h"

#include "Core.RTTI/MetaType.Definitions-inl.h"

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const Symbol* InsertSymbol_(
    Symbols::hashmap_type& symbols,
    Symbol::TypeId type, const StringSlice& cstr, u64 ord ) {
    Assert(not cstr.empty());

    const Symbol* result = &(symbols[cstr] = Symbol(type, cstr, ord));

    forrange(i, 1, cstr.size()) {
        const StringSlice prefix = cstr.CutBefore(i);
        Symbol& symbol = symbols[prefix];
        if (not symbol.IsValid()) {
            symbol = Symbol(Symbol::TypeId(Symbol::Prefix|symbol.Type()|type), prefix);
            Assert(symbol.IsPrefix());
            Assert(not symbol.IsValid());
        }
    }

    Assert(result);
    Assert(result->Type() == type);
    Assert(result->CStr() == cstr);
    Assert(result->Ord() == ord);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
static void RegisterSymbol_(
    const Symbol** psymbol,
    typename Symbols::hashmap_type& symbols,
    typename Symbol::TypeId type, const char (&cstr)[_Dim], u64 ord = 0) {
    Assert(nullptr == *psymbol);
    *psymbol = InsertSymbol_(symbols, type, MakeStringSlice(cstr), ord);
}
//----------------------------------------------------------------------------
static void RegisterRTTITypenames_(Symbols::hashmap_type& symbols) {
#define RTTI_INSERT_TYPENAME(_Name, T, _TypeId, _Unused) \
    InsertSymbol_(symbols, Symbol::Typename, MakeStringSlice(STRINGIZE(_Name)), _TypeId);
    FOREACH_CORE_RTTI_NATIVE_TYPES(RTTI_INSERT_TYPENAME)
#undef RTTI_INSERT_TYPENAME
}
//----------------------------------------------------------------------------
static const Symbol* FindSymbol_(const Symbols::hashmap_type& symbols, const StringSlice& cstr) {
    const auto it = symbols.find(cstr);
    return (symbols.end() != it ? &it->second : nullptr);
}
//----------------------------------------------------------------------------
static void UnregisterSymbol_(const Symbols::hashmap_type& symbols, const Symbol** psymbol) {
    Assert(nullptr != *psymbol);
    Assert(*psymbol == FindSymbol_(symbols, (*psymbol)->CStr()));
    *psymbol = nullptr;
}
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
static void CheckSymbol_(const Symbols::hashmap_type& symbols, const Symbol* symbol) {
    Assert(symbol);
    const Symbol* stored = FindSymbol_(symbols, symbol->CStr());
    Assert(stored == symbol);
    Assert(symbol->IsValid());
    Assert(not symbol->IsPrefix());
}
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Symbols::IsPrefix(const Symbol** psymbol, const StringSlice& cstr) const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(psymbol);
    Assert(not cstr.empty());

    const auto it = _symbols.find(cstr);

    if (it == _symbols.end()) {
        *psymbol = nullptr;
        return false;
    }
    else {
        *psymbol = &it->second;
        return true;
    }
}
//----------------------------------------------------------------------------
Symbols::Symbols() {

    _symbols.reserve(256);

    RegisterSymbol_(&True, _symbols, Symbol::True, "true");
    RegisterSymbol_(&False, _symbols, Symbol::False, "false");
    RegisterSymbol_(&Nil, _symbols, Symbol::Nil, "nil");
    RegisterSymbol_(&Is, _symbols, Symbol::Is, "is");
    RegisterSymbol_(&Extern, _symbols, Symbol::Extern, "extern");
    RegisterSymbol_(&Export, _symbols, Symbol::Export, "export");
    RegisterSymbol_(&LBrace, _symbols, Symbol::LBrace, "{");
    RegisterSymbol_(&RBrace, _symbols, Symbol::RBrace, "}");
    RegisterSymbol_(&LBracket, _symbols, Symbol::LBracket, "[");
    RegisterSymbol_(&RBracket, _symbols, Symbol::RBracket, "]");
    RegisterSymbol_(&LParenthese, _symbols, Symbol::LParenthese, "(");
    RegisterSymbol_(&RParenthese, _symbols, Symbol::RParenthese, ")");
    RegisterSymbol_(&Comma, _symbols, Symbol::Comma, ",");
    RegisterSymbol_(&Colon, _symbols, Symbol::Colon, ":");
    RegisterSymbol_(&SemiColon, _symbols, Symbol::SemiColon, ";");
    RegisterSymbol_(&Dot, _symbols, Symbol::Dot, ".");
    RegisterSymbol_(&Dollar, _symbols, Symbol::Dollar, "$");
    RegisterSymbol_(&Question, _symbols, Symbol::Question, "?");
    RegisterSymbol_(&Add, _symbols, Symbol::Add, "+");
    RegisterSymbol_(&Sub, _symbols, Symbol::Sub, "-");
    RegisterSymbol_(&Mul, _symbols, Symbol::Mul, "*");
    RegisterSymbol_(&Div, _symbols, Symbol::Div, "/");
    RegisterSymbol_(&Mod, _symbols, Symbol::Mod, "%");
    RegisterSymbol_(&Pow, _symbols, Symbol::Pow, "**");
    RegisterSymbol_(&Increment, _symbols, Symbol::Increment, "++");
    RegisterSymbol_(&Decrement, _symbols, Symbol::Decrement, "--");
    RegisterSymbol_(&LShift, _symbols, Symbol::LShift, "<<");
    RegisterSymbol_(&RShift, _symbols, Symbol::RShift, ">>");
    RegisterSymbol_(&And, _symbols, Symbol::And, "&");
    RegisterSymbol_(&Or, _symbols, Symbol::Or, "|");
    RegisterSymbol_(&Not, _symbols, Symbol::Not, "!");
    RegisterSymbol_(&Xor, _symbols, Symbol::Xor, "^");
    RegisterSymbol_(&Complement, _symbols, Symbol::Complement, "~");
    RegisterSymbol_(&Assignment, _symbols, Symbol::Assignment, "=");
    RegisterSymbol_(&Equals, _symbols, Symbol::Equals, "==");
    RegisterSymbol_(&NotEquals, _symbols, Symbol::NotEquals, "!=");
    RegisterSymbol_(&Less, _symbols, Symbol::Less, "<");
    RegisterSymbol_(&LessOrEqual, _symbols, Symbol::LessOrEqual, "<=");
    RegisterSymbol_(&Greater, _symbols, Symbol::Greater, ">");
    RegisterSymbol_(&GreaterOrEqual, _symbols, Symbol::GreaterOrEqual, ">=");
    RegisterSymbol_(&DotDot, _symbols, Symbol::DotDot, "..");

    Symbols::Invalid = new Symbol(Symbol::Invalid, "%invalid%");
    Symbols::Eof = new Symbol(Symbol::Eof, MakeStringSlice("Eof"));
    Symbols::Int = new Symbol(Symbol::Int, MakeStringSlice("Int"));
    Symbols::Float = new Symbol(Symbol::Float, MakeStringSlice("Float"));
    Symbols::String = new Symbol(Symbol::String, MakeStringSlice("String"));
    Symbols::Identifier = new Symbol(Symbol::Identifier, MakeStringSlice("Identifier"));
    Symbols::Typename = new Symbol(Symbol::Typename, MakeStringSlice("Typename"));

    RegisterRTTITypenames_(_symbols);

#ifdef WITH_CORE_ASSERT
    CheckSymbol_(_symbols, Symbols::True);
    CheckSymbol_(_symbols, Symbols::False);
    CheckSymbol_(_symbols, Symbols::Nil);
    CheckSymbol_(_symbols, Symbols::Is);
    CheckSymbol_(_symbols, Symbols::Extern);
    CheckSymbol_(_symbols, Symbols::Export);
    CheckSymbol_(_symbols, Symbols::LBrace);
    CheckSymbol_(_symbols, Symbols::RBrace);
    CheckSymbol_(_symbols, Symbols::LBracket);
    CheckSymbol_(_symbols, Symbols::RBracket);
    CheckSymbol_(_symbols, Symbols::LParenthese);
    CheckSymbol_(_symbols, Symbols::RParenthese);
    CheckSymbol_(_symbols, Symbols::Comma);
    CheckSymbol_(_symbols, Symbols::Colon);
    CheckSymbol_(_symbols, Symbols::SemiColon);
    CheckSymbol_(_symbols, Symbols::Dot);
    CheckSymbol_(_symbols, Symbols::Dollar);
    CheckSymbol_(_symbols, Symbols::Question);
    CheckSymbol_(_symbols, Symbols::Add);
    CheckSymbol_(_symbols, Symbols::Sub);
    CheckSymbol_(_symbols, Symbols::Mul);
    CheckSymbol_(_symbols, Symbols::Div);
    CheckSymbol_(_symbols, Symbols::Mod);
    CheckSymbol_(_symbols, Symbols::Pow);
    CheckSymbol_(_symbols, Symbols::Increment);
    CheckSymbol_(_symbols, Symbols::Decrement);
    CheckSymbol_(_symbols, Symbols::LShift);
    CheckSymbol_(_symbols, Symbols::RShift);
    CheckSymbol_(_symbols, Symbols::And);
    CheckSymbol_(_symbols, Symbols::Or);
    CheckSymbol_(_symbols, Symbols::Not);
    CheckSymbol_(_symbols, Symbols::Xor);
    CheckSymbol_(_symbols, Symbols::Complement);
    CheckSymbol_(_symbols, Symbols::Assignment);
    CheckSymbol_(_symbols, Symbols::Equals);
    CheckSymbol_(_symbols, Symbols::NotEquals);
    CheckSymbol_(_symbols, Symbols::Less);
    CheckSymbol_(_symbols, Symbols::LessOrEqual);
    CheckSymbol_(_symbols, Symbols::Greater);
    CheckSymbol_(_symbols, Symbols::GreaterOrEqual);
    CheckSymbol_(_symbols, Symbols::DotDot);
#endif
}
//----------------------------------------------------------------------------
Symbols::~Symbols() {
    UnregisterSymbol_(_symbols, &True);
    UnregisterSymbol_(_symbols, &False);
    UnregisterSymbol_(_symbols, &Nil);
    UnregisterSymbol_(_symbols, &Is);
    UnregisterSymbol_(_symbols, &Extern);
    UnregisterSymbol_(_symbols, &Export);
    UnregisterSymbol_(_symbols, &LBrace);
    UnregisterSymbol_(_symbols, &RBrace);
    UnregisterSymbol_(_symbols, &LBracket);
    UnregisterSymbol_(_symbols, &RBracket);
    UnregisterSymbol_(_symbols, &LParenthese);
    UnregisterSymbol_(_symbols, &RParenthese);
    UnregisterSymbol_(_symbols, &Comma);
    UnregisterSymbol_(_symbols, &Colon);
    UnregisterSymbol_(_symbols, &SemiColon);
    UnregisterSymbol_(_symbols, &Dot);
    UnregisterSymbol_(_symbols, &Dollar);
    UnregisterSymbol_(_symbols, &Question);
    UnregisterSymbol_(_symbols, &Add);
    UnregisterSymbol_(_symbols, &Sub);
    UnregisterSymbol_(_symbols, &Mul);
    UnregisterSymbol_(_symbols, &Div);
    UnregisterSymbol_(_symbols, &Mod);
    UnregisterSymbol_(_symbols, &Pow);
    UnregisterSymbol_(_symbols, &Increment);
    UnregisterSymbol_(_symbols, &Decrement);
    UnregisterSymbol_(_symbols, &LShift);
    UnregisterSymbol_(_symbols, &RShift);
    UnregisterSymbol_(_symbols, &And);
    UnregisterSymbol_(_symbols, &Or);
    UnregisterSymbol_(_symbols, &Not);
    UnregisterSymbol_(_symbols, &Xor);
    UnregisterSymbol_(_symbols, &Complement);
    UnregisterSymbol_(_symbols, &Assignment);
    UnregisterSymbol_(_symbols, &Equals);
    UnregisterSymbol_(_symbols, &NotEquals);
    UnregisterSymbol_(_symbols, &Less);
    UnregisterSymbol_(_symbols, &LessOrEqual);
    UnregisterSymbol_(_symbols, &Greater);
    UnregisterSymbol_(_symbols, &GreaterOrEqual);
    UnregisterSymbol_(_symbols, &DotDot);

    Assert(nullptr != Symbols::Invalid);
    Assert(nullptr != Symbols::Eof);
    Assert(nullptr != Symbols::Int);
    Assert(nullptr != Symbols::Float);
    Assert(nullptr != Symbols::String);
    Assert(nullptr != Symbols::Identifier);
    Assert(nullptr != Symbols::Typename);

    checked_delete(Symbols::Invalid);
    checked_delete(Symbols::Eof);
    checked_delete(Symbols::Int);
    checked_delete(Symbols::Float);
    checked_delete(Symbols::String);
    checked_delete(Symbols::Identifier);
    checked_delete(Symbols::Typename);

    Invalid = nullptr;
    Eof = nullptr;
    Int = nullptr;
    Float = nullptr;
    String = nullptr;
    Identifier = nullptr;
    Typename = nullptr;
}
//----------------------------------------------------------------------------
const Symbol *Symbols::Invalid = nullptr;
const Symbol *Symbols::Eof = nullptr;
const Symbol *Symbols::Int = nullptr;
const Symbol *Symbols::Float = nullptr;
const Symbol *Symbols::String = nullptr;
const Symbol *Symbols::Identifier = nullptr;
const Symbol *Symbols::True = nullptr;
const Symbol *Symbols::False = nullptr;
const Symbol *Symbols::Nil = nullptr;
const Symbol *Symbols::Is = nullptr;
const Symbol *Symbols::Extern = nullptr;
const Symbol *Symbols::Export = nullptr;
const Symbol *Symbols::LBrace = nullptr;
const Symbol *Symbols::RBrace = nullptr;
const Symbol *Symbols::LBracket = nullptr;
const Symbol *Symbols::RBracket = nullptr;
const Symbol *Symbols::LParenthese = nullptr;
const Symbol *Symbols::RParenthese = nullptr;
const Symbol *Symbols::Comma = nullptr;
const Symbol *Symbols::Colon = nullptr;
const Symbol *Symbols::SemiColon = nullptr;
const Symbol *Symbols::Dot = nullptr;
const Symbol *Symbols::Dollar = nullptr;
const Symbol *Symbols::Question = nullptr;
const Symbol *Symbols::Add = nullptr;
const Symbol *Symbols::Sub = nullptr;
const Symbol *Symbols::Mul = nullptr;
const Symbol *Symbols::Div = nullptr;
const Symbol *Symbols::Mod = nullptr;
const Symbol *Symbols::Pow = nullptr;
const Symbol *Symbols::Increment = nullptr;
const Symbol *Symbols::Decrement = nullptr;
const Symbol *Symbols::LShift = nullptr;
const Symbol *Symbols::RShift = nullptr;
const Symbol *Symbols::And = nullptr;
const Symbol *Symbols::Or = nullptr;
const Symbol *Symbols::Not = nullptr;
const Symbol *Symbols::Xor = nullptr;
const Symbol *Symbols::Complement = nullptr;
const Symbol *Symbols::Assignment = nullptr;
const Symbol *Symbols::Equals = nullptr;
const Symbol *Symbols::NotEquals = nullptr;
const Symbol *Symbols::Less = nullptr;
const Symbol *Symbols::LessOrEqual = nullptr;
const Symbol *Symbols::Greater = nullptr;
const Symbol *Symbols::GreaterOrEqual = nullptr;
const Symbol *Symbols::DotDot = nullptr;
const Symbol *Symbols::Typename = nullptr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
