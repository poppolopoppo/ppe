#include "stdafx.h"

#include "Parser.h"
#include "ParseExpression.h"

#include "MetaClass.h"
#include "MetaClassDatabase.h"
#include "MetaObject.h"
#include "MetaProperty.h"

#include "Format.h"
#include "PoolAllocator-impl.h"

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
SINGLETON_POOL_ALLOCATED_DEF(VariableExport, )
//----------------------------------------------------------------------------
VariableExport::VariableExport(const RTTI::MetaObjectName& name, const PCParseExpression& value, const Flags scope, const Lexer::Location& site)
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
        context->AddLocal(_name, atom.get());
        break;

    case VariableExport::Global:
        context->AddGlobal(_name, atom.get());
        break;

    default:
        Assert(false);
        break;
    }

    return atom.get();
}
//----------------------------------------------------------------------------
String VariableExport::ToString() const {
    return _name.cstr();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(VariableReference, )
//----------------------------------------------------------------------------
VariableReference::VariableReference(const RTTI::MetaObjectName& name, const Lexer::Location& site)
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
        throw ParserException("unknown variable name", this);

    return value;
}
//----------------------------------------------------------------------------
String VariableReference::ToString() const {
    return _name.cstr();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(ObjectDefinition, )
//----------------------------------------------------------------------------
ObjectDefinition::ObjectDefinition(const RTTI::MetaClassName& name, const Lexer::Location& site)
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
        throw ParserException("unknown metaclass", this);

    const RTTI::PMetaObject obj = metaclass->CreateInstance();
    Assert(obj);

    const RTTI::PMetaObject parent = context->ScopeObject();
    context->SetScopeObject(obj.get());

    for (const PCParseStatement& statement : _statements)
        statement->Execute(context);

    context->SetScopeObject(parent.get());

    return MakeAtom(obj);
}
//----------------------------------------------------------------------------
String ObjectDefinition::ToString() const {
    return _name.cstr();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(PropertyReference, )
//----------------------------------------------------------------------------
PropertyReference::PropertyReference(
    const PCParseExpression& object,
    const RTTI::MetaPropertyName& member,
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
        throw ParserException("invalid object", _object.get());

    const RTTI::PMetaObject object = typed_atom->Wrapper();
    if (!object)
        throw ParserException("object is null", _object.get());

    const RTTI::MetaClass *metaclass = object->RTTI_MetaClass();
    Assert(metaclass);

    const RTTI::MetaProperty *metaproperty = metaclass->PropertyIFP(_member);
    if (!metaproperty)
        throw ParserException("invalid member name", this);

    RTTI::MetaAtom *const pvalue = metaproperty->WrapCopy(object.get());

    return pvalue;
}
//----------------------------------------------------------------------------
String PropertyReference::ToString() const {
    return StringFormat("{0}::{1}", _object->ToString(), _member.cstr());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(Pair, )
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
SINGLETON_POOL_ALLOCATED_DEF(Array, )
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
SINGLETON_POOL_ALLOCATED_DEF(Dictionary, )
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
} //!namespace Parser
} //!namespace Core
