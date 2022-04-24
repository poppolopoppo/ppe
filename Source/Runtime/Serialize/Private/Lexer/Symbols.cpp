#include "stdafx.h"

#include "Lexer/Symbols.h"

#include "Lexer/Symbol.h"

#include "MetaObject.h"
#include "RTTI/NativeTypes.h"

namespace PPE {
namespace Lexer {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const FSymbol* InsertSymbol_(
    FSymbols::hashmap_type& symbols,
    FSymbol::ETypeId type, const FStringView& cstr, u64 ord ) {
    Assert(not cstr.empty());

    const FSymbol* result = &(symbols[cstr] = FSymbol(type, cstr, ord));

    forrange(i, 1, cstr.size()) {
        const FStringView prefix = cstr.CutBefore(i);
        FSymbol& symbol = symbols[prefix];
        if (not symbol.IsValid()) {
            symbol = FSymbol(FSymbol::ETypeId(FSymbol::Prefix|symbol.Type()|type), prefix);
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
    const FSymbol** psymbol,
    typename FSymbols::hashmap_type& symbols,
    typename FSymbol::ETypeId type, const char (&cstr)[_Dim], u64 ord = 0) {
    Assert(nullptr == *psymbol);
    *psymbol = InsertSymbol_(symbols, type, MakeStringView(cstr), ord);
}
//----------------------------------------------------------------------------
static void RegisterRTTITypenames_(FSymbols::hashmap_type& symbols) {
#define RTTI_INSERT_TYPENAME(_Name, T, _TypeId) \
    InsertSymbol_(symbols, FSymbol::Typename, MakeStringView(STRINGIZE(_Name)), _TypeId);
    FOREACH_RTTI_NATIVETYPES(RTTI_INSERT_TYPENAME)
#undef RTTI_INSERT_TYPENAME
}
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
static const FSymbol* FindSymbol_(const FSymbols::hashmap_type& symbols, const FStringView& cstr) {
    const auto it = symbols.find(cstr);
    return (symbols.end() != it ? &it->second : nullptr);
}
#endif
//----------------------------------------------------------------------------
static void UnregisterSymbol_(const FSymbols::hashmap_type& symbols, const FSymbol** psymbol) {
    Unused(symbols);
    Assert(nullptr != *psymbol);
    Assert_NoAssume(*psymbol == FindSymbol_(symbols, (*psymbol)->CStr()));
    *psymbol = nullptr;
}
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
static void CheckSymbol_(const FSymbols::hashmap_type& symbols, const FSymbol* symbol) {
    Assert(symbol);
    const FSymbol* stored = FindSymbol_(symbols, symbol->CStr());
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
void* FSymbols::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared libs
}
//----------------------------------------------------------------------------
bool FSymbols::IsPrefix(const FSymbol** psymbol, const FStringView& cstr) const {
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
FSymbols::FSymbols() {

    _symbols.reserve(256);

    RegisterSymbol_(&True, _symbols, FSymbol::True, "true");
    RegisterSymbol_(&False, _symbols, FSymbol::False, "false");
    RegisterSymbol_(&Null, _symbols, FSymbol::Null, "null");
    RegisterSymbol_(&Is, _symbols, FSymbol::Is, "is");
    RegisterSymbol_(&Extern, _symbols, FSymbol::Extern, "extern");
    RegisterSymbol_(&Export, _symbols, FSymbol::Export, "export");
    RegisterSymbol_(&LBrace, _symbols, FSymbol::LBrace, "{");
    RegisterSymbol_(&RBrace, _symbols, FSymbol::RBrace, "}");
    RegisterSymbol_(&LBracket, _symbols, FSymbol::LBracket, "[");
    RegisterSymbol_(&RBracket, _symbols, FSymbol::RBracket, "]");
    RegisterSymbol_(&LParenthese, _symbols, FSymbol::LParenthese, "(");
    RegisterSymbol_(&RParenthese, _symbols, FSymbol::RParenthese, ")");
    RegisterSymbol_(&Comma, _symbols, FSymbol::Comma, ",");
    RegisterSymbol_(&Colon, _symbols, FSymbol::Colon, ":");
    RegisterSymbol_(&SemiColon, _symbols, FSymbol::SemiColon, ";");
    RegisterSymbol_(&Dot, _symbols, FSymbol::Dot, ".");
    RegisterSymbol_(&Dollar, _symbols, FSymbol::Dollar, "$");
    RegisterSymbol_(&Question, _symbols, FSymbol::Question, "?");
    RegisterSymbol_(&Add, _symbols, FSymbol::Add, "+");
    RegisterSymbol_(&Sub, _symbols, FSymbol::Sub, "-");
    RegisterSymbol_(&Mul, _symbols, FSymbol::Mul, "*");
    RegisterSymbol_(&Div, _symbols, FSymbol::Div, "/");
    RegisterSymbol_(&Mod, _symbols, FSymbol::Mod, "%");
    RegisterSymbol_(&Pow, _symbols, FSymbol::Pow, "**");
    RegisterSymbol_(&Increment, _symbols, FSymbol::Increment, "++");
    RegisterSymbol_(&Decrement, _symbols, FSymbol::Decrement, "--");
    RegisterSymbol_(&LShift, _symbols, FSymbol::LShift, "<<");
    RegisterSymbol_(&RShift, _symbols, FSymbol::RShift, ">>");
    RegisterSymbol_(&And, _symbols, FSymbol::And, "&");
    RegisterSymbol_(&Or, _symbols, FSymbol::Or, "|");
    RegisterSymbol_(&Not, _symbols, FSymbol::Not, "!");
    RegisterSymbol_(&Xor, _symbols, FSymbol::Xor, "^");
    RegisterSymbol_(&Complement, _symbols, FSymbol::Complement, "~");
    RegisterSymbol_(&Assignment, _symbols, FSymbol::Assignment, "=");
    RegisterSymbol_(&Equals, _symbols, FSymbol::Equals, "==");
    RegisterSymbol_(&NotEquals, _symbols, FSymbol::NotEquals, "!=");
    RegisterSymbol_(&Less, _symbols, FSymbol::Less, "<");
    RegisterSymbol_(&LessOrEqual, _symbols, FSymbol::LessOrEqual, "<=");
    RegisterSymbol_(&Greater, _symbols, FSymbol::Greater, ">");
    RegisterSymbol_(&GreaterOrEqual, _symbols, FSymbol::GreaterOrEqual, ">=");
    RegisterSymbol_(&DotDot, _symbols, FSymbol::DotDot, "..");
    RegisterSymbol_(&Sharp, _symbols, FSymbol::Sharp, "#");

    FSymbols::Invalid = new FSymbol(FSymbol::Invalid, "%invalid%");
    FSymbols::Eof = new FSymbol(FSymbol::Eof, "Eof");
    FSymbols::Integer = new FSymbol(FSymbol::Integer, "Integer");
    FSymbols::Unsigned = new FSymbol(FSymbol::Unsigned, "Unsigned");
    FSymbols::Float = new FSymbol(FSymbol::Float, "Float");
    FSymbols::String = new FSymbol(FSymbol::String, "String");
    FSymbols::Identifier = new FSymbol(FSymbol::Identifier, "Identifier");
    FSymbols::Typename = new FSymbol(FSymbol::Typename, "Typename");

    RegisterRTTITypenames_(_symbols);

#if USE_PPE_ASSERT
    CheckSymbol_(_symbols, FSymbols::True);
    CheckSymbol_(_symbols, FSymbols::False);
    CheckSymbol_(_symbols, FSymbols::Null);
    CheckSymbol_(_symbols, FSymbols::Is);
    CheckSymbol_(_symbols, FSymbols::Extern);
    CheckSymbol_(_symbols, FSymbols::Export);
    CheckSymbol_(_symbols, FSymbols::LBrace);
    CheckSymbol_(_symbols, FSymbols::RBrace);
    CheckSymbol_(_symbols, FSymbols::LBracket);
    CheckSymbol_(_symbols, FSymbols::RBracket);
    CheckSymbol_(_symbols, FSymbols::LParenthese);
    CheckSymbol_(_symbols, FSymbols::RParenthese);
    CheckSymbol_(_symbols, FSymbols::Comma);
    CheckSymbol_(_symbols, FSymbols::Colon);
    CheckSymbol_(_symbols, FSymbols::SemiColon);
    CheckSymbol_(_symbols, FSymbols::Dot);
    CheckSymbol_(_symbols, FSymbols::Dollar);
    CheckSymbol_(_symbols, FSymbols::Question);
    CheckSymbol_(_symbols, FSymbols::Add);
    CheckSymbol_(_symbols, FSymbols::Sub);
    CheckSymbol_(_symbols, FSymbols::Mul);
    CheckSymbol_(_symbols, FSymbols::Div);
    CheckSymbol_(_symbols, FSymbols::Mod);
    CheckSymbol_(_symbols, FSymbols::Pow);
    CheckSymbol_(_symbols, FSymbols::Increment);
    CheckSymbol_(_symbols, FSymbols::Decrement);
    CheckSymbol_(_symbols, FSymbols::LShift);
    CheckSymbol_(_symbols, FSymbols::RShift);
    CheckSymbol_(_symbols, FSymbols::And);
    CheckSymbol_(_symbols, FSymbols::Or);
    CheckSymbol_(_symbols, FSymbols::Not);
    CheckSymbol_(_symbols, FSymbols::Xor);
    CheckSymbol_(_symbols, FSymbols::Complement);
    CheckSymbol_(_symbols, FSymbols::Assignment);
    CheckSymbol_(_symbols, FSymbols::Equals);
    CheckSymbol_(_symbols, FSymbols::NotEquals);
    CheckSymbol_(_symbols, FSymbols::Less);
    CheckSymbol_(_symbols, FSymbols::LessOrEqual);
    CheckSymbol_(_symbols, FSymbols::Greater);
    CheckSymbol_(_symbols, FSymbols::GreaterOrEqual);
    CheckSymbol_(_symbols, FSymbols::DotDot);
    CheckSymbol_(_symbols, FSymbols::Sharp);
#endif
}
//----------------------------------------------------------------------------
FSymbols::~FSymbols() {
    UnregisterSymbol_(_symbols, &True);
    UnregisterSymbol_(_symbols, &False);
    UnregisterSymbol_(_symbols, &Null);
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
    UnregisterSymbol_(_symbols, &Sharp);

    Assert(nullptr != FSymbols::Invalid);
    Assert(nullptr != FSymbols::Eof);
    Assert(nullptr != FSymbols::Integer);
    Assert(nullptr != FSymbols::Unsigned);
    Assert(nullptr != FSymbols::Float);
    Assert(nullptr != FSymbols::String);
    Assert(nullptr != FSymbols::Identifier);
    Assert(nullptr != FSymbols::Typename);

    checked_delete_ref(FSymbols::Invalid);
    checked_delete_ref(FSymbols::Eof);
    checked_delete_ref(FSymbols::Integer);
    checked_delete_ref(FSymbols::Unsigned);
    checked_delete_ref(FSymbols::Float);
    checked_delete_ref(FSymbols::String);
    checked_delete_ref(FSymbols::Identifier);
    checked_delete_ref(FSymbols::Typename);
}
//----------------------------------------------------------------------------
const FSymbol *FSymbols::Invalid = nullptr;
const FSymbol *FSymbols::Eof = nullptr;
const FSymbol *FSymbols::Integer = nullptr;
const FSymbol *FSymbols::Unsigned = nullptr;
const FSymbol *FSymbols::Float = nullptr;
const FSymbol *FSymbols::String = nullptr;
const FSymbol *FSymbols::Identifier = nullptr;
const FSymbol *FSymbols::True = nullptr;
const FSymbol *FSymbols::False = nullptr;
const FSymbol *FSymbols::Null = nullptr;
const FSymbol *FSymbols::Is = nullptr;
const FSymbol *FSymbols::Extern = nullptr;
const FSymbol *FSymbols::Export = nullptr;
const FSymbol *FSymbols::LBrace = nullptr;
const FSymbol *FSymbols::RBrace = nullptr;
const FSymbol *FSymbols::LBracket = nullptr;
const FSymbol *FSymbols::RBracket = nullptr;
const FSymbol *FSymbols::LParenthese = nullptr;
const FSymbol *FSymbols::RParenthese = nullptr;
const FSymbol *FSymbols::Comma = nullptr;
const FSymbol *FSymbols::Colon = nullptr;
const FSymbol *FSymbols::SemiColon = nullptr;
const FSymbol *FSymbols::Dot = nullptr;
const FSymbol *FSymbols::Dollar = nullptr;
const FSymbol *FSymbols::Question = nullptr;
const FSymbol *FSymbols::Add = nullptr;
const FSymbol *FSymbols::Sub = nullptr;
const FSymbol *FSymbols::Mul = nullptr;
const FSymbol *FSymbols::Div = nullptr;
const FSymbol *FSymbols::Mod = nullptr;
const FSymbol *FSymbols::Pow = nullptr;
const FSymbol *FSymbols::Increment = nullptr;
const FSymbol *FSymbols::Decrement = nullptr;
const FSymbol *FSymbols::LShift = nullptr;
const FSymbol *FSymbols::RShift = nullptr;
const FSymbol *FSymbols::And = nullptr;
const FSymbol *FSymbols::Or = nullptr;
const FSymbol *FSymbols::Not = nullptr;
const FSymbol *FSymbols::Xor = nullptr;
const FSymbol *FSymbols::Complement = nullptr;
const FSymbol *FSymbols::Assignment = nullptr;
const FSymbol *FSymbols::Equals = nullptr;
const FSymbol *FSymbols::NotEquals = nullptr;
const FSymbol *FSymbols::Less = nullptr;
const FSymbol *FSymbols::LessOrEqual = nullptr;
const FSymbol *FSymbols::Greater = nullptr;
const FSymbol *FSymbols::GreaterOrEqual = nullptr;
const FSymbol *FSymbols::DotDot = nullptr;
const FSymbol *FSymbols::Sharp = nullptr;
const FSymbol *FSymbols::Typename = nullptr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lexer
} //!namespace PPE
