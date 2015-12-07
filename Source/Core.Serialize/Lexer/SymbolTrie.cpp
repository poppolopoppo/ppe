#include "stdafx.h"

#include "SymbolTrie.h"

#include "Core/Meta/Singleton.h"

namespace Core {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class SymbolTrieSingleton : Meta::Singleton<SymbolTrie, SymbolTrieSingleton> {
public:
    typedef Meta::Singleton<SymbolTrie, SymbolTrieSingleton> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create() {
        parent_type::Create();
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SymbolTrie::SymbolTrie() {
    SymbolTrie::True = Insert_(Symbol::True, "true");
    SymbolTrie::False = Insert_(Symbol::False, "false");
    SymbolTrie::Nil = Insert_(Symbol::Nil, "nil");
    SymbolTrie::Is = Insert_(Symbol::Is, "is");
    SymbolTrie::Extern = Insert_(Symbol::Extern, "extern");
    SymbolTrie::Export = Insert_(Symbol::Export, "export");
    SymbolTrie::LBrace = Insert_(Symbol::LBrace, "{");
    SymbolTrie::RBrace = Insert_(Symbol::RBrace, "}");
    SymbolTrie::LBracket = Insert_(Symbol::LBracket, "[");
    SymbolTrie::RBracket = Insert_(Symbol::RBracket, "]");
    SymbolTrie::LParenthese = Insert_(Symbol::LParenthese, "(");
    SymbolTrie::RParenthese = Insert_(Symbol::RParenthese, ")");
    SymbolTrie::Comma = Insert_(Symbol::Comma, ",");
    SymbolTrie::Colon = Insert_(Symbol::Colon, ":");
    SymbolTrie::SemiColon = Insert_(Symbol::SemiColon, ";");
    SymbolTrie::Dot = Insert_(Symbol::Dot, ".");
    SymbolTrie::Dollar = Insert_(Symbol::Dollar, "$");
    SymbolTrie::Question = Insert_(Symbol::Question, "?");
    SymbolTrie::Add = Insert_(Symbol::Add, "+");
    SymbolTrie::Sub = Insert_(Symbol::Sub, "-");
    SymbolTrie::Mul = Insert_(Symbol::Mul, "*");
    SymbolTrie::Div = Insert_(Symbol::Div, "/");
    SymbolTrie::Mod = Insert_(Symbol::Mod, "%");
    SymbolTrie::Pow = Insert_(Symbol::Pow, "**");
    SymbolTrie::Increment = Insert_(Symbol::Increment, "++");
    SymbolTrie::Decrement = Insert_(Symbol::Decrement, "--");
    SymbolTrie::LShift = Insert_(Symbol::LShift, "<<");
    SymbolTrie::RShift = Insert_(Symbol::RShift, ">>");
    SymbolTrie::And = Insert_(Symbol::And, "&");
    SymbolTrie::Or = Insert_(Symbol::Or, "|");
    SymbolTrie::Not = Insert_(Symbol::Not, "!");
    SymbolTrie::Xor = Insert_(Symbol::Xor, "^");
    SymbolTrie::Complement = Insert_(Symbol::Complement, "~");
    SymbolTrie::Assignment = Insert_(Symbol::Assignment, "=");
    SymbolTrie::Equals = Insert_(Symbol::Equals, "==");
    SymbolTrie::NotEquals = Insert_(Symbol::NotEquals, "!=");
    SymbolTrie::Less = Insert_(Symbol::Less, "<");
    SymbolTrie::LessOrEqual = Insert_(Symbol::LessOrEqual, "<=");
    SymbolTrie::Greater = Insert_(Symbol::Greater, ">");
    SymbolTrie::GreaterOrEqual = Insert_(Symbol::GreaterOrEqual, ">=");
    SymbolTrie::DotDot = Insert_(Symbol::DotDot, "..");

    Root()->Emplace(Symbol::Invalid, StringSlice());
    SymbolTrie::Invalid = &Root()->Value();

    SymbolTrie::Eof = new Symbol(Symbol::Eof, MakeStringSlice("Eof"));

    SymbolTrie::Int = new Symbol(Symbol::Int, MakeStringSlice("Int"));
    SymbolTrie::Float = new Symbol(Symbol::Float, MakeStringSlice("Float"));
    SymbolTrie::String = new Symbol(Symbol::String, MakeStringSlice("String"));
    SymbolTrie::Identifier = new Symbol(Symbol::Identifier, MakeStringSlice("Identifier"));
}
//----------------------------------------------------------------------------
SymbolTrie::~SymbolTrie() {
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

    SymbolTrie::Invalid = nullptr;

    checked_delete(SymbolTrie::Eof);
    SymbolTrie::Eof = nullptr;

    checked_delete(SymbolTrie::Int);
    checked_delete(SymbolTrie::Float);
    checked_delete(SymbolTrie::String);
    checked_delete(SymbolTrie::Identifier);

    SymbolTrie::Int = nullptr;
    SymbolTrie::Float = nullptr;
    SymbolTrie::String = nullptr;
    SymbolTrie::Identifier = nullptr;
}
//----------------------------------------------------------------------------
const Symbol *SymbolTrie::Insert_(Symbol::TypeId type, const char *cstr, size_t size) {
    const StringSlice cstr_slice(cstr, size);
    return &parent_type::Insert_AssertUnique(cstr_slice, Symbol(type, cstr_slice))->Value();
}
//----------------------------------------------------------------------------
void SymbolTrie::Create() {
    SymbolTrieSingleton::Create();
}
//----------------------------------------------------------------------------
void SymbolTrie::Destroy() {
    SymbolTrieSingleton::Destroy();
}
//----------------------------------------------------------------------------
bool SymbolTrie::HasInstance() {
    return SymbolTrieSingleton::HasInstance();
}
//----------------------------------------------------------------------------
const SymbolTrie& SymbolTrie::Instance() {
    return SymbolTrieSingleton::Instance();
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
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace Core
