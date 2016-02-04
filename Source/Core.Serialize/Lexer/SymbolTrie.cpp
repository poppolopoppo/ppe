#include "stdafx.h"

#include "SymbolTrie.h"

#include "Core.RTTI/MetaType.Definitions-inl.h"

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const Symbol *Insert_(
        SymbolMap& symbols,
        Symbol::TypeId type,
        const char *cstr, size_t size,
        u64 ord ) {
    const StringSlice cstr_slice(cstr, size);
    SymbolMap::node_type* node = symbols.Insert_AssertUnique(cstr_slice);
    Assert(Symbol::Invalid == node->Value().Type());
    node->SetValue(Symbol(type, cstr_slice, ord));
    return &node->Value();
}
//----------------------------------------------------------------------------
template <size_t _Dim>
static const Symbol *Insert_(SymbolMap& symbols, Symbol::TypeId type, const char (&cstr)[_Dim], u64 ord = 0) {
    return Insert_(symbols, type, cstr, _Dim - 1, ord);
}
//----------------------------------------------------------------------------
static void CheckSymbol_(const SymbolMap& symbols, const Symbol* symbol) {
    Assert(symbol);
    const SymbolMap::node_type* node = symbols.Find(symbol->CStr());
    Assert(node);
    Assert(node->HasValue());
    Assert(node->Value() == *symbol);
}
//----------------------------------------------------------------------------
static void RTTI_InsertTypenames_(SymbolMap& symbols) {
#define RTTI_INSERT_TYPENAME(_Name, T, _TypeId, _Unused) \
    Insert_(symbols, Symbol::Typename, STRINGIZE(_Name), _TypeId);
    FOREACH_CORE_RTTI_NATIVE_TYPES(RTTI_INSERT_TYPENAME)
#undef RTTI_INSERT_TYPENAME
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
auto SymbolTrie::IsPrefix(const StringSlice& cstr, const query_t& hint /* = nullptr */) -> query_t {
    Assert(!cstr.empty());

    const SymbolMap& symbols = singleton_type::Instance().Map();
    SymbolMap::query_t result = symbols.Find(cstr, hint.Node ? &hint : nullptr);

    return result;
}
//----------------------------------------------------------------------------
void SymbolTrie::Create() {
    singleton_type::Create();

    SymbolMap& symbols = singleton_type::Instance().Map();

    SymbolTrie::True = Insert_(symbols, Symbol::True, "true");
    SymbolTrie::False = Insert_(symbols, Symbol::False, "false");
    SymbolTrie::Nil = Insert_(symbols, Symbol::Nil, "nil");
    SymbolTrie::Is = Insert_(symbols, Symbol::Is, "is");
    SymbolTrie::Extern = Insert_(symbols, Symbol::Extern, "extern");
    SymbolTrie::Export = Insert_(symbols, Symbol::Export, "export");
    SymbolTrie::LBrace = Insert_(symbols, Symbol::LBrace, "{");
    SymbolTrie::RBrace = Insert_(symbols, Symbol::RBrace, "}");
    SymbolTrie::LBracket = Insert_(symbols, Symbol::LBracket, "[");
    SymbolTrie::RBracket = Insert_(symbols, Symbol::RBracket, "]");
    SymbolTrie::LParenthese = Insert_(symbols, Symbol::LParenthese, "(");
    SymbolTrie::RParenthese = Insert_(symbols, Symbol::RParenthese, ")");
    SymbolTrie::Comma = Insert_(symbols, Symbol::Comma, ",");
    SymbolTrie::Colon = Insert_(symbols, Symbol::Colon, ":");
    SymbolTrie::SemiColon = Insert_(symbols, Symbol::SemiColon, ";");
    SymbolTrie::Dot = Insert_(symbols, Symbol::Dot, ".");
    SymbolTrie::Dollar = Insert_(symbols, Symbol::Dollar, "$");
    SymbolTrie::Question = Insert_(symbols, Symbol::Question, "?");
    SymbolTrie::Add = Insert_(symbols, Symbol::Add, "+");
    SymbolTrie::Sub = Insert_(symbols, Symbol::Sub, "-");
    SymbolTrie::Mul = Insert_(symbols, Symbol::Mul, "*");
    SymbolTrie::Div = Insert_(symbols, Symbol::Div, "/");
    SymbolTrie::Mod = Insert_(symbols, Symbol::Mod, "%");
    SymbolTrie::Pow = Insert_(symbols, Symbol::Pow, "**");
    SymbolTrie::Increment = Insert_(symbols, Symbol::Increment, "++");
    SymbolTrie::Decrement = Insert_(symbols, Symbol::Decrement, "--");
    SymbolTrie::LShift = Insert_(symbols, Symbol::LShift, "<<");
    SymbolTrie::RShift = Insert_(symbols, Symbol::RShift, ">>");
    SymbolTrie::And = Insert_(symbols, Symbol::And, "&");
    SymbolTrie::Or = Insert_(symbols, Symbol::Or, "|");
    SymbolTrie::Not = Insert_(symbols, Symbol::Not, "!");
    SymbolTrie::Xor = Insert_(symbols, Symbol::Xor, "^");
    SymbolTrie::Complement = Insert_(symbols, Symbol::Complement, "~");
    SymbolTrie::Assignment = Insert_(symbols, Symbol::Assignment, "=");
    SymbolTrie::Equals = Insert_(symbols, Symbol::Equals, "==");
    SymbolTrie::NotEquals = Insert_(symbols, Symbol::NotEquals, "!=");
    SymbolTrie::Less = Insert_(symbols, Symbol::Less, "<");
    SymbolTrie::LessOrEqual = Insert_(symbols, Symbol::LessOrEqual, "<=");
    SymbolTrie::Greater = Insert_(symbols, Symbol::Greater, ">");
    SymbolTrie::GreaterOrEqual = Insert_(symbols, Symbol::GreaterOrEqual, ">=");
    SymbolTrie::DotDot = Insert_(symbols, Symbol::DotDot, "..");

    SymbolTrie::Invalid = new Symbol(Symbol::Invalid, "%invalid%");

    SymbolTrie::Eof = new Symbol(Symbol::Eof, MakeStringSlice("Eof"));

    SymbolTrie::Int = new Symbol(Symbol::Int, MakeStringSlice("Int"));
    SymbolTrie::Float = new Symbol(Symbol::Float, MakeStringSlice("Float"));
    SymbolTrie::String = new Symbol(Symbol::String, MakeStringSlice("String"));
    SymbolTrie::Identifier = new Symbol(Symbol::Identifier, MakeStringSlice("Identifier"));

    SymbolTrie::Typename = new Symbol(Symbol::Typename, MakeStringSlice("Typename"));

    RTTI_InsertTypenames_(symbols);

#ifdef WITH_CORE_ASSERT
    CheckSymbol_(symbols, SymbolTrie::True);
    CheckSymbol_(symbols, SymbolTrie::False);
    CheckSymbol_(symbols, SymbolTrie::Nil);
    CheckSymbol_(symbols, SymbolTrie::Is);
    CheckSymbol_(symbols, SymbolTrie::Extern);
    CheckSymbol_(symbols, SymbolTrie::Export);
    CheckSymbol_(symbols, SymbolTrie::LBrace);
    CheckSymbol_(symbols, SymbolTrie::RBrace);
    CheckSymbol_(symbols, SymbolTrie::LBracket);
    CheckSymbol_(symbols, SymbolTrie::RBracket);
    CheckSymbol_(symbols, SymbolTrie::LParenthese);
    CheckSymbol_(symbols, SymbolTrie::RParenthese);
    CheckSymbol_(symbols, SymbolTrie::Comma);
    CheckSymbol_(symbols, SymbolTrie::Colon);
    CheckSymbol_(symbols, SymbolTrie::SemiColon);
    CheckSymbol_(symbols, SymbolTrie::Dot);
    CheckSymbol_(symbols, SymbolTrie::Dollar);
    CheckSymbol_(symbols, SymbolTrie::Question);
    CheckSymbol_(symbols, SymbolTrie::Add);
    CheckSymbol_(symbols, SymbolTrie::Sub);
    CheckSymbol_(symbols, SymbolTrie::Mul);
    CheckSymbol_(symbols, SymbolTrie::Div);
    CheckSymbol_(symbols, SymbolTrie::Mod);
    CheckSymbol_(symbols, SymbolTrie::Pow);
    CheckSymbol_(symbols, SymbolTrie::Increment);
    CheckSymbol_(symbols, SymbolTrie::Decrement);
    CheckSymbol_(symbols, SymbolTrie::LShift);
    CheckSymbol_(symbols, SymbolTrie::RShift);
    CheckSymbol_(symbols, SymbolTrie::And);
    CheckSymbol_(symbols, SymbolTrie::Or);
    CheckSymbol_(symbols, SymbolTrie::Not);
    CheckSymbol_(symbols, SymbolTrie::Xor);
    CheckSymbol_(symbols, SymbolTrie::Complement);
    CheckSymbol_(symbols, SymbolTrie::Assignment);
    CheckSymbol_(symbols, SymbolTrie::Equals);
    CheckSymbol_(symbols, SymbolTrie::NotEquals);
    CheckSymbol_(symbols, SymbolTrie::Less);
    CheckSymbol_(symbols, SymbolTrie::LessOrEqual);
    CheckSymbol_(symbols, SymbolTrie::Greater);
    CheckSymbol_(symbols, SymbolTrie::GreaterOrEqual);
    CheckSymbol_(symbols, SymbolTrie::DotDot);
#endif
}
//----------------------------------------------------------------------------
void SymbolTrie::Destroy() {

    SymbolTrie::True = nullptr;
    SymbolTrie::False = nullptr;
    SymbolTrie::Nil = nullptr;
    SymbolTrie::Is = nullptr;
    SymbolTrie::Extern = nullptr;
    SymbolTrie::Export = nullptr;
    SymbolTrie::LBrace = nullptr;
    SymbolTrie::RBrace = nullptr;
    SymbolTrie::LBracket = nullptr;
    SymbolTrie::RBracket = nullptr;
    SymbolTrie::LParenthese = nullptr;
    SymbolTrie::RParenthese = nullptr;
    SymbolTrie::Comma = nullptr;
    SymbolTrie::Colon = nullptr;
    SymbolTrie::SemiColon = nullptr;
    SymbolTrie::Dot = nullptr;
    SymbolTrie::Dollar = nullptr;
    SymbolTrie::Question = nullptr;
    SymbolTrie::Add = nullptr;
    SymbolTrie::Sub = nullptr;
    SymbolTrie::Mul = nullptr;
    SymbolTrie::Div = nullptr;
    SymbolTrie::Mod = nullptr;
    SymbolTrie::Pow = nullptr;
    SymbolTrie::Increment = nullptr;
    SymbolTrie::Decrement = nullptr;
    SymbolTrie::LShift = nullptr;
    SymbolTrie::RShift = nullptr;
    SymbolTrie::And = nullptr;
    SymbolTrie::Or = nullptr;
    SymbolTrie::Not = nullptr;
    SymbolTrie::Xor = nullptr;
    SymbolTrie::Complement = nullptr;
    SymbolTrie::Assignment = nullptr;
    SymbolTrie::Equals = nullptr;
    SymbolTrie::NotEquals = nullptr;
    SymbolTrie::Less = nullptr;
    SymbolTrie::LessOrEqual = nullptr;
    SymbolTrie::Greater = nullptr;
    SymbolTrie::GreaterOrEqual = nullptr;
    SymbolTrie::DotDot = nullptr;

    checked_delete(SymbolTrie::Invalid);
    SymbolTrie::Invalid = nullptr;

    checked_delete(SymbolTrie::Eof);
    SymbolTrie::Eof = nullptr;

    checked_delete(SymbolTrie::Int);
    checked_delete(SymbolTrie::Float);
    checked_delete(SymbolTrie::String);
    checked_delete(SymbolTrie::Identifier);
    checked_delete(SymbolTrie::Typename);

    SymbolTrie::Int = nullptr;
    SymbolTrie::Float = nullptr;
    SymbolTrie::String = nullptr;
    SymbolTrie::Identifier = nullptr;
    SymbolTrie::Typename = nullptr;

    singleton_type::Destroy();
}
//----------------------------------------------------------------------------
const Symbol *SymbolTrie::Invalid = nullptr;

const Symbol *SymbolTrie::Eof = nullptr;

const Symbol *SymbolTrie::Int = nullptr;
const Symbol *SymbolTrie::Float = nullptr;
const Symbol *SymbolTrie::String = nullptr;
const Symbol *SymbolTrie::Identifier = nullptr;

const Symbol *SymbolTrie::True = nullptr;
const Symbol *SymbolTrie::False = nullptr;
const Symbol *SymbolTrie::Nil = nullptr;
const Symbol *SymbolTrie::Is = nullptr;
const Symbol *SymbolTrie::Extern = nullptr;
const Symbol *SymbolTrie::Export = nullptr;
const Symbol *SymbolTrie::LBrace = nullptr;
const Symbol *SymbolTrie::RBrace = nullptr;
const Symbol *SymbolTrie::LBracket = nullptr;
const Symbol *SymbolTrie::RBracket = nullptr;
const Symbol *SymbolTrie::LParenthese = nullptr;
const Symbol *SymbolTrie::RParenthese = nullptr;
const Symbol *SymbolTrie::Comma = nullptr;
const Symbol *SymbolTrie::Colon = nullptr;
const Symbol *SymbolTrie::SemiColon = nullptr;
const Symbol *SymbolTrie::Dot = nullptr;
const Symbol *SymbolTrie::Dollar = nullptr;
const Symbol *SymbolTrie::Question = nullptr;
const Symbol *SymbolTrie::Add = nullptr;
const Symbol *SymbolTrie::Sub = nullptr;
const Symbol *SymbolTrie::Mul = nullptr;
const Symbol *SymbolTrie::Div = nullptr;
const Symbol *SymbolTrie::Mod = nullptr;
const Symbol *SymbolTrie::Pow = nullptr;
const Symbol *SymbolTrie::Increment = nullptr;
const Symbol *SymbolTrie::Decrement = nullptr;
const Symbol *SymbolTrie::LShift = nullptr;
const Symbol *SymbolTrie::RShift = nullptr;
const Symbol *SymbolTrie::And = nullptr;
const Symbol *SymbolTrie::Or = nullptr;
const Symbol *SymbolTrie::Not = nullptr;
const Symbol *SymbolTrie::Xor = nullptr;
const Symbol *SymbolTrie::Complement = nullptr;
const Symbol *SymbolTrie::Assignment = nullptr;
const Symbol *SymbolTrie::Equals = nullptr;
const Symbol *SymbolTrie::NotEquals = nullptr;
const Symbol *SymbolTrie::Less = nullptr;
const Symbol *SymbolTrie::LessOrEqual = nullptr;
const Symbol *SymbolTrie::Greater = nullptr;
const Symbol *SymbolTrie::GreaterOrEqual = nullptr;
const Symbol *SymbolTrie::DotDot = nullptr;

const Symbol *SymbolTrie::Typename = nullptr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
