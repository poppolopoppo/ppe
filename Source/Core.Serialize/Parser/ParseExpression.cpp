#include "stdafx.h"

#include "Parser.h"

#include "ParseExpression.h"

#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaDatabase.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/IO/Format.h"

#include <sstream>

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FParseExpression::FParseExpression(const Lexer::FLocation& site)
:   FParseItem(site) {}
//----------------------------------------------------------------------------
FParseExpression::~FParseExpression() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FVariableExport, )
//----------------------------------------------------------------------------
FVariableExport::FVariableExport(const RTTI::FName& name, const PCParseExpression& value, const EFlags scope, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _name(name), _value(value), _scope(scope) {
    Assert(!name.empty());
    Assert(value);
}
//----------------------------------------------------------------------------
FVariableExport::~FVariableExport() {}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *FVariableExport::Eval(FParseContext *context) const {
    Assert(context);

    const RTTI::PMetaAtom atom = _value->Eval(context);

    switch (_scope)
    {
    case FVariableExport::Public:
        context->AddLocal(this, _name, atom.get());
        break;

    case FVariableExport::Global:
        context->AddGlobal(this, _name, atom.get());
        break;

    default:
        Assert(false);
        break;
    }

    return atom.get();
}
//----------------------------------------------------------------------------
FString FVariableExport::ToString() const {
    return _name.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FVariableReference, )
//----------------------------------------------------------------------------
FVariableReference::FVariableReference(const RTTI::FName& name, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _name(name) {
    Assert(!name.empty());
}
//----------------------------------------------------------------------------
FVariableReference::~FVariableReference() {}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *FVariableReference::Eval(FParseContext *context) const {
    Assert(context);

    RTTI::FMetaAtom *const value = context->GetAny(_name);
    if (!value)
        CORE_THROW_IT(FParserException("unknown variable name", this));

    return value;
}
//----------------------------------------------------------------------------
FString FVariableReference::ToString() const {
    return _name.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FObjectDefinition, )
//----------------------------------------------------------------------------
FObjectDefinition::FObjectDefinition(const RTTI::FName& name, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _name(name) {
    Assert(!name.empty());
}
//----------------------------------------------------------------------------
FObjectDefinition::~FObjectDefinition() {}
//----------------------------------------------------------------------------
void FObjectDefinition::AddStatement(const FParseStatement *statement) {
    Assert(statement);

    _statements.emplace_back(statement);
}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *FObjectDefinition::Eval(FParseContext *context) const {
    Assert(context);

    const RTTI::FMetaClass *metaclass = RTTI::MetaDB().FindClassIFP(_name);
    if (!metaclass)
        CORE_THROW_IT(FParserException("unknown metaclass", this));

    const RTTI::PMetaObject obj = metaclass->CreateInstance();
    Assert(obj);

    const RTTI::PMetaObject parent = context->ScopeObject();
    context->SetScopeObject(obj.get());

    for (const PCParseStatement& statement : _statements)
        statement->Execute(context);

    context->SetScopeObject(parent.get());

    return RTTI::MakeAtom(obj);
}
//----------------------------------------------------------------------------
FString FObjectDefinition::ToString() const {
    return _name.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FPropertyReference, )
//----------------------------------------------------------------------------
FPropertyReference::FPropertyReference(
    const PCParseExpression& object,
    const RTTI::FName& member,
    const Lexer::FLocation& site)
:   FParseExpression(site),
    _object(object), _member(member) {
    Assert(object);
    Assert(!member.empty());
}
//----------------------------------------------------------------------------
FPropertyReference::~FPropertyReference() {}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *FPropertyReference::Eval(FParseContext *context) const {
    Assert(context);

    const RTTI::PCMetaAtom atom = _object->Eval(context);
    Assert(atom);

    const auto *typed_atom = atom->As<RTTI::PMetaObject>();
    if (!typed_atom)
        CORE_THROW_IT(FParserException("invalid object", _object.get()));

    const RTTI::PMetaObject object = typed_atom->Wrapper();
    if (!object)
        CORE_THROW_IT(FParserException("object is null", _object.get()));

    const RTTI::FMetaClass *metaclass = object->RTTI_MetaClass();
    Assert(metaclass);

    const RTTI::FMetaProperty *metaproperty = metaclass->PropertyIFP(_member);
    if (!metaproperty)
        CORE_THROW_IT(FParserException("invalid member name", this));

    RTTI::FMetaAtom *const pvalue = metaproperty->WrapCopy(object.get());

    return pvalue;
}
//----------------------------------------------------------------------------
FString FPropertyReference::ToString() const {
    return StringFormat("{0}::{1}", _object->ToString(), _member.c_str());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, TPair, )
//----------------------------------------------------------------------------
TPair::TPair(
    const PCParseExpression& lhs,
    const PCParseExpression& rhs,
    const Lexer::FLocation& site)
:   FParseExpression(site),
    _lhs(lhs), _rhs(rhs) {
    Assert(lhs);
    Assert(rhs);
}
//----------------------------------------------------------------------------
TPair::~TPair() {}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *TPair::Eval(FParseContext *context) const {
    Assert(context);

    RTTI::PMetaAtom lhs_atom = _lhs->Eval(context);
    Assert(lhs_atom);
    RTTI::PMetaAtom rhs_atom = _rhs->Eval(context);
    Assert(rhs_atom);

    Core::TPair<RTTI::PMetaAtom, RTTI::PMetaAtom> pair(lhs_atom, rhs_atom);

    return RTTI::MakeAtom(pair);
}
//----------------------------------------------------------------------------
FString TPair::ToString() const {
    return StringFormat("({0}, {1})", _lhs->ToString(), _rhs->ToString());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, TArray, )
//----------------------------------------------------------------------------
TArray::TArray(const Lexer::FLocation& site)
:   FParseExpression(site) {}
//----------------------------------------------------------------------------
TArray::TArray(
    const TMemoryView<const PCParseExpression>& items,
    const Lexer::FLocation& site)
:   FParseExpression(site) {
    _items.insert(_items.end(), items.begin(), items.end());
}
//----------------------------------------------------------------------------
TArray::~TArray() {}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *TArray::Eval(FParseContext *context) const {
    Assert(context);

    VECTOR(Parser, RTTI::PMetaAtom) atom_items;
    atom_items.reserve(_items.size());

    for (const auto& it : _items)
        atom_items.push_back(it->Eval(context));

    return RTTI::MakeAtom(atom_items);
}
//----------------------------------------------------------------------------
FString TArray::ToString() const {
    FOStringStream oss;
    oss << "TArray[ ";
    for (const auto& it : _items)
        oss << it->ToString() << ", ";
    oss << "]";
    return oss.str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, TDictionary, )
//----------------------------------------------------------------------------
TDictionary::TDictionary(const Lexer::FLocation& site)
:   FParseExpression(site) {}
//----------------------------------------------------------------------------
TDictionary::TDictionary(
    const TMemoryView<const Core::TPair<PCParseExpression, PCParseExpression>>& items,
    const Lexer::FLocation& site)
:   FParseExpression(site) {
    _items.insert(items.begin(), items.end());
}
//----------------------------------------------------------------------------
TDictionary::~TDictionary() {}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *TDictionary::Eval(FParseContext *context) const {
    Assert(context);

    ASSOCIATIVE_VECTOR(Parser, RTTI::PMetaAtom, RTTI::PMetaAtom) atom_items;
    atom_items.reserve(_items.size());

    for (const auto& it : _items) {
        const RTTI::PMetaAtom key = it.first->Eval(context);
        const RTTI::PMetaAtom value = it.second->Eval(context);
        atom_items.Insert_AssertUnique(std::move(key), std::move(value));
    }

    return RTTI::MakeAtom(atom_items);
}
//----------------------------------------------------------------------------
FString TDictionary::ToString() const {
    FOStringStream oss;
    oss << "TDictionary{ ";
    for (const auto& it : _items)
        oss << it.first->ToString() << " => " << it.second->ToString() << ", ";
    oss << "}";
    return oss.str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FCastExpr, )
//----------------------------------------------------------------------------
FCastExpr::FCastExpr(RTTI::FMetaTypeId typeId, const FParseExpression* expr, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _typeId(typeId)
,   _expr(expr) {
    Assert(_typeId);
    Assert(_expr);
}
//----------------------------------------------------------------------------
FCastExpr::~FCastExpr() {}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *FCastExpr::Eval(FParseContext *context) const {
    Assert(context);

    RTTI::PMetaAtom atom = _expr->Eval(context);
    if (nullptr == atom)
        return nullptr;

    RTTI::PMetaAtom cast = RTTI::ScalarTraitsFromTypeId(_typeId)->CreateDefaultValue();
    Assert(cast);

    if (false == cast->Traits()->AssignMove(cast.get(), atom.get()))
        CORE_THROW_IT(FParserException("invalid cast", this));

    return RemoveRef_AssertReachZero_KeepAlive(cast);
}
//----------------------------------------------------------------------------
FString FCastExpr::ToString() const {
    FOStringStream oss;
    oss << RTTI::ScalarTypeInfoFromTypeId(_typeId).Name << ": ";
    oss << _expr->ToString();
    return oss.str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
