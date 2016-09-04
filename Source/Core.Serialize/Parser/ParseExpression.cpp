#include "stdafx.h"

#include "Parser.h"

#include "ParseExpression.h"

#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaClassDatabase.h"
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
ParseExpression::ParseExpression(const Lexer::Location& site)
:   ParseItem(site) {}
//----------------------------------------------------------------------------
ParseExpression::~ParseExpression() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, VariableExport, )
//----------------------------------------------------------------------------
VariableExport::VariableExport(const RTTI::Name& name, const PCParseExpression& value, const Flags scope, const Lexer::Location& site)
:   ParseExpression(site)
,   _name(name), _value(value), _scope(scope) {
    Assert(!name.empty());
    Assert(value);
}
//----------------------------------------------------------------------------
VariableExport::~VariableExport() {}
//----------------------------------------------------------------------------
RTTI::MetaAtom *VariableExport::Eval(ParseContext *context) const {
    Assert(context);

    const RTTI::PMetaAtom atom = _value->Eval(context);

    switch (_scope)
    {
    case VariableExport::Public:
        context->AddLocal(this, _name, atom.get());
        break;

    case VariableExport::Global:
        context->AddGlobal(this, _name, atom.get());
        break;

    default:
        Assert(false);
        break;
    }

    return atom.get();
}
//----------------------------------------------------------------------------
String VariableExport::ToString() const {
    return _name.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, VariableReference, )
//----------------------------------------------------------------------------
VariableReference::VariableReference(const RTTI::Name& name, const Lexer::Location& site)
:   ParseExpression(site)
,   _name(name) {
    Assert(!name.empty());
}
//----------------------------------------------------------------------------
VariableReference::~VariableReference() {}
//----------------------------------------------------------------------------
RTTI::MetaAtom *VariableReference::Eval(ParseContext *context) const {
    Assert(context);

    RTTI::MetaAtom *const value = context->GetAny(_name);
    if (!value)
        CORE_THROW_IT(ParserException("unknown variable name", this));

    return value;
}
//----------------------------------------------------------------------------
String VariableReference::ToString() const {
    return _name.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, ObjectDefinition, )
//----------------------------------------------------------------------------
ObjectDefinition::ObjectDefinition(const RTTI::Name& name, const Lexer::Location& site)
:   ParseExpression(site)
,   _name(name) {
    Assert(!name.empty());
}
//----------------------------------------------------------------------------
ObjectDefinition::~ObjectDefinition() {}
//----------------------------------------------------------------------------
void ObjectDefinition::AddStatement(const ParseStatement *statement) {
    Assert(statement);

    _statements.emplace_back(statement);
}
//----------------------------------------------------------------------------
RTTI::MetaAtom *ObjectDefinition::Eval(ParseContext *context) const {
    Assert(context);

    const RTTI::MetaClass *metaclass = RTTI::MetaClassDatabase::Instance().GetIFP(_name);
    if (!metaclass)
        CORE_THROW_IT(ParserException("unknown metaclass", this));

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
String ObjectDefinition::ToString() const {
    return _name.c_str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, PropertyReference, )
//----------------------------------------------------------------------------
PropertyReference::PropertyReference(
    const PCParseExpression& object,
    const RTTI::Name& member,
    const Lexer::Location& site)
:   ParseExpression(site),
    _object(object), _member(member) {
    Assert(object);
    Assert(!member.empty());
}
//----------------------------------------------------------------------------
PropertyReference::~PropertyReference() {}
//----------------------------------------------------------------------------
RTTI::MetaAtom *PropertyReference::Eval(ParseContext *context) const {
    Assert(context);

    const RTTI::PCMetaAtom atom = _object->Eval(context);
    Assert(atom);

    const auto *typed_atom = atom->As<RTTI::PMetaObject>();
    if (!typed_atom)
        CORE_THROW_IT(ParserException("invalid object", _object.get()));

    const RTTI::PMetaObject object = typed_atom->Wrapper();
    if (!object)
        CORE_THROW_IT(ParserException("object is null", _object.get()));

    const RTTI::MetaClass *metaclass = object->RTTI_MetaClass();
    Assert(metaclass);

    const RTTI::MetaProperty *metaproperty = metaclass->PropertyIFP(_member);
    if (!metaproperty)
        CORE_THROW_IT(ParserException("invalid member name", this));

    RTTI::MetaAtom *const pvalue = metaproperty->WrapCopy(object.get());

    return pvalue;
}
//----------------------------------------------------------------------------
String PropertyReference::ToString() const {
    return StringFormat("{0}::{1}", _object->ToString(), _member.c_str());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, Pair, )
//----------------------------------------------------------------------------
Pair::Pair(
    const PCParseExpression& lhs,
    const PCParseExpression& rhs,
    const Lexer::Location& site)
:   ParseExpression(site),
    _lhs(lhs), _rhs(rhs) {
    Assert(lhs);
    Assert(rhs);
}
//----------------------------------------------------------------------------
Pair::~Pair() {}
//----------------------------------------------------------------------------
RTTI::MetaAtom *Pair::Eval(ParseContext *context) const {
    Assert(context);

    RTTI::PMetaAtom lhs_atom = _lhs->Eval(context);
    Assert(lhs_atom);
    RTTI::PMetaAtom rhs_atom = _rhs->Eval(context);
    Assert(rhs_atom);

    Core::Pair<RTTI::PMetaAtom, RTTI::PMetaAtom> pair(lhs_atom, rhs_atom);

    return RTTI::MakeAtom(pair);
}
//----------------------------------------------------------------------------
String Pair::ToString() const {
    return StringFormat("({0}, {1})", _lhs->ToString(), _rhs->ToString());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, Array, )
//----------------------------------------------------------------------------
Array::Array(const Lexer::Location& site)
:   ParseExpression(site) {}
//----------------------------------------------------------------------------
Array::Array(
    const MemoryView<const PCParseExpression>& items,
    const Lexer::Location& site)
:   ParseExpression(site) {
    _items.insert(_items.end(), items.begin(), items.end());
}
//----------------------------------------------------------------------------
Array::~Array() {}
//----------------------------------------------------------------------------
RTTI::MetaAtom *Array::Eval(ParseContext *context) const {
    Assert(context);

    VECTOR(Parser, RTTI::PMetaAtom) atom_items;
    atom_items.reserve(_items.size());

    for (const auto& it : _items)
        atom_items.push_back(it->Eval(context));

    return RTTI::MakeAtom(atom_items);
}
//----------------------------------------------------------------------------
String Array::ToString() const {
    OStringStream oss;
    oss << "Array[ ";
    for (const auto& it : _items)
        oss << it->ToString() << ", ";
    oss << "]";
    return oss.str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, Dictionary, )
//----------------------------------------------------------------------------
Dictionary::Dictionary(const Lexer::Location& site)
:   ParseExpression(site) {}
//----------------------------------------------------------------------------
Dictionary::Dictionary(
    const MemoryView<const Core::Pair<PCParseExpression, PCParseExpression>>& items,
    const Lexer::Location& site)
:   ParseExpression(site) {
    _items.insert(items.begin(), items.end());
}
//----------------------------------------------------------------------------
Dictionary::~Dictionary() {}
//----------------------------------------------------------------------------
RTTI::MetaAtom *Dictionary::Eval(ParseContext *context) const {
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
String Dictionary::ToString() const {
    OStringStream oss;
    oss << "Dictionary{ ";
    for (const auto& it : _items)
        oss << it.first->ToString() << " => " << it.second->ToString() << ", ";
    oss << "}";
    return oss.str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, CastExpr, )
//----------------------------------------------------------------------------
CastExpr::CastExpr(RTTI::MetaTypeId typeId, const ParseExpression* expr, const Lexer::Location& site)
:   ParseExpression(site)
,   _typeId(typeId)
,   _expr(expr) {
    Assert(_typeId);
    Assert(_expr);
}
//----------------------------------------------------------------------------
CastExpr::~CastExpr() {}
//----------------------------------------------------------------------------
RTTI::MetaAtom *CastExpr::Eval(ParseContext *context) const {
    Assert(context);

    RTTI::PMetaAtom atom = _expr->Eval(context);
    if (nullptr == atom)
        return nullptr;

    RTTI::PMetaAtom cast = RTTI::ScalarTraitsFromTypeId(_typeId)->CreateDefaultValue();
    Assert(cast);

    if (false == cast->Traits()->AssignMove(cast.get(), atom.get()))
        CORE_THROW_IT(ParserException("invalid cast", this));

    return RemoveRef_AssertReachZero_KeepAlive(cast);
}
//----------------------------------------------------------------------------
String CastExpr::ToString() const {
    OStringStream oss;
    oss << RTTI::ScalarTypeInfoFromTypeId(_typeId).Name << ": ";
    oss << _expr->ToString();
    return oss.str();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
