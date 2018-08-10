#include "stdafx.h"

#include "Parser/ParseExpression.h"

#include "Parser/Parser.h"
#include "Parser/ParseStatement.h"

#include "Any.h"
#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaProperty.h"

#include "Allocator/PoolAllocator-impl.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"

namespace PPE {
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
RTTI::FAtom FVariableExport::Eval(FParseContext* context) const {
    Assert(context);

    const RTTI::FAtom atom = _value->Eval(context);

    switch (_scope) {
    case FVariableExport::Public:
        context->AddLocal(this, _name, atom);
        break;

    case FVariableExport::Global:
        context->AddGlobal(this, _name, atom);
        break;

    default:
        Assert(false);
        break;
    }

    return atom;
}
//----------------------------------------------------------------------------
FString FVariableExport::ToString() const {
    return FString(_name.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FVariableReference, )
//----------------------------------------------------------------------------
FVariableReference::FVariableReference(const RTTI::FPathName& pathName, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _pathName(pathName) {
    Assert(not _pathName.empty());
}
//----------------------------------------------------------------------------
FVariableReference::~FVariableReference() {}
//----------------------------------------------------------------------------
RTTI::FAtom FVariableReference::Eval(FParseContext* context) const {
    Assert(context);

    RTTI::FAtom value;

    if (_pathName.Transaction.empty()) { // local ?
        value = context->GetAny(_pathName.Identifier);

        if (not value)
            PPE_THROW_IT(FParserException("unknown variable name", this));
    }
    else { // global :
        RTTI::PMetaObject objIFP{ RTTI::MetaDB().ObjectIFP(_pathName) };

        if (not objIFP)
            PPE_THROW_IT(FParserException("unknown object path", this));

        value = context->CreateAtomFrom(std::move(objIFP));
    }

    Assert(value);
    return value;
}
//----------------------------------------------------------------------------
FString FVariableReference::ToString() const {
    return PPE::ToString(_pathName);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FObjectDefinition, )
//----------------------------------------------------------------------------
FObjectDefinition::FObjectDefinition(const RTTI::FName& name, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _name(name) {
    Assert(not name.empty());
}
//----------------------------------------------------------------------------
FObjectDefinition::~FObjectDefinition() {}
//----------------------------------------------------------------------------
void FObjectDefinition::AddStatement(const FParseStatement *statement) {
    Assert(statement);

    _statements.emplace_back(statement);
}
//----------------------------------------------------------------------------
RTTI::FAtom FObjectDefinition::Eval(FParseContext* context) const {
    Assert(context);

    const RTTI::FMetaClass *metaclass = RTTI::MetaDB().ClassIFP(_name);
    if (not metaclass)
        PPE_THROW_IT(FParserException("unknown metaclass", this));

    RTTI::PMetaObject obj;
    Verify(metaclass->CreateInstance(obj));
    Assert(obj);

    const RTTI::PMetaObject parent = context->ScopeObject();
    context->SetScopeObject(obj.get());

    for (const PCParseStatement& statement : _statements)
        statement->Execute(context);

    context->SetScopeObject(parent.get());

    return context->CreateAtomFrom(std::move(obj));
}
//----------------------------------------------------------------------------
FString FObjectDefinition::ToString() const {
    return FString(_name.MakeView());
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
RTTI::FAtom FPropertyReference::Eval(FParseContext* context) const {
    Assert(context);

    const RTTI::FAtom atom = _object->Eval(context);
    Assert(atom);

    const RTTI::PMetaObject* ppobj = atom.TypedDataIFP<RTTI::PMetaObject>();
    if (not ppobj)
        PPE_THROW_IT(FParserException("invalid object", _object.get()));

    const RTTI::PMetaObject pobj = (*ppobj);
    if (not pobj)
        PPE_THROW_IT(FParserException("object is null", _object.get()));

    const RTTI::FMetaClass* metaclass = pobj->RTTI_Class();
    Assert(metaclass);

    const RTTI::FMetaProperty* metaproperty = metaclass->PropertyIFP(_member);
    if (not metaproperty)
        PPE_THROW_IT(FParserException("invalid member name", this));

    return metaproperty->Get(*pobj);
}
//----------------------------------------------------------------------------
FString FPropertyReference::ToString() const {
    return StringFormat("{0}::{1}", _object->ToString(), _member.c_str());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FTupleExpr, )
//----------------------------------------------------------------------------
FTupleExpr::FTupleExpr(const Lexer::FLocation& site)
:   FParseExpression(site)
{}
//----------------------------------------------------------------------------
FTupleExpr::FTupleExpr(elements_type&& relements, const Lexer::FLocation& site)
    : FParseExpression(site)
    , _elements(std::move(relements)) {
#ifdef WITH_PPE_ASSERT
    Assert(_elements.size() > 1);
    for (const auto& expr : _elements)
        Assert(expr);
#endif
}
//----------------------------------------------------------------------------
FTupleExpr::~FTupleExpr() {}
//----------------------------------------------------------------------------
RTTI::FAtom FTupleExpr::Eval(FParseContext* context) const {
    Assert(context);

    const RTTI::PTypeTraits traits = RTTI::MakeAnyTuple(_elements.size());
    const RTTI::ITupleTraits& tupleTraits = traits->ToTuple();

    const RTTI::FAtom result = context->CreateAtom(traits);

    forrange(i, 0, _elements.size()) {
        const auto& expr = _elements[i];

        const RTTI::FAtom src = expr->Eval(context);
        Assert(src);
        const RTTI::FAtom dst = tupleTraits.At(result.Data(), i);
        Assert(dst);

        dst.FlatData<RTTI::FAny>().AssignMove(src);
    }

    return result;
}
//----------------------------------------------------------------------------
FString FTupleExpr::ToString() const {
    FStringBuilder oss;
    auto sep = Fmt::NotFirstTime(", ");
    oss << "Tuple:( ";
    for (const auto& elt : _elements)
        oss << sep << elt->ToString();
    oss << " )";
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FArrayExpr, )
//----------------------------------------------------------------------------
FArrayExpr::FArrayExpr(const Lexer::FLocation& site)
:   FParseExpression(site) {}
//----------------------------------------------------------------------------
FArrayExpr::FArrayExpr(items_type&& ritems, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _items(std::move(ritems))
{}
//----------------------------------------------------------------------------
FArrayExpr::~FArrayExpr() {}
//----------------------------------------------------------------------------
RTTI::FAtom FArrayExpr::Eval(FParseContext* context) const {
    Assert(context);

    using any_vector = VECTOR_LINEARHEAP(RTTI::FAny);

    any_vector result{ context->CreateHeapContainer<any_vector>() };
    result.resize(_items.size());

    forrange(i, 0, _items.size()) {
        const RTTI::FAtom atom = _items[i]->Eval(context);
        Assert(atom);

        result[i].AssignMove(atom);
    }

    return context->CreateAtomFrom(std::move(result));
}
//----------------------------------------------------------------------------
FString FArrayExpr::ToString() const {
    FStringBuilder oss;
    auto sep = Fmt::NotFirstTime(", ");
    oss << "Array:[ ";
    for (const auto& it : _items)
        oss << sep << it->ToString();
    oss << " ]";
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FDictionaryExpr, )
//----------------------------------------------------------------------------
FDictionaryExpr::FDictionaryExpr(const Lexer::FLocation& site)
:   FParseExpression(site) {}
//----------------------------------------------------------------------------
FDictionaryExpr::FDictionaryExpr(dico_type&& rdico, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _dico(std::move(rdico))
{}
//----------------------------------------------------------------------------
FDictionaryExpr::~FDictionaryExpr() {}
//----------------------------------------------------------------------------
RTTI::FAtom FDictionaryExpr::Eval(FParseContext* context) const {
    Assert(context);

    using any_dico = ASSOCIATIVE_VECTOR_LINEARHEAP(RTTI::FAny, RTTI::FAny);

    any_dico result{ context->CreateHeapContainer<any_dico>() };
    result.Vector().resize(_dico.size());

    forrange(i, 0, _dico.size()) {
        const auto& it = _dico.Vector()[i];
        const RTTI::FAtom key = it.first->Eval(context);
        const RTTI::FAtom value = it.second->Eval(context);

        Assert(key);

        PPE::TPair<RTTI::FAny, RTTI::FAny>& pair = result.Vector()[i];

        pair.first.AssignMove(key);
        pair.second.AssignMove(value);
    }

    return context->CreateAtomFrom(std::move(result));
}
//----------------------------------------------------------------------------
FString FDictionaryExpr::ToString() const {
    FStringBuilder oss;
    auto sep = Fmt::NotFirstTime(", ");
    oss << "DictionaryExpr:{ ";
    for (const auto& it : _dico)
        oss << sep << it.first->ToString() << " => " << it.second->ToString() << ", ";
    oss << " }";
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Parser, FCastExpr, )
//----------------------------------------------------------------------------
FCastExpr::FCastExpr(RTTI::ENativeType typeId, const FParseExpression* expr, const Lexer::FLocation& site)
:   FParseExpression(site)
,   _typeId(typeId)
,   _expr(expr) {
    Assert(_expr);
}
//----------------------------------------------------------------------------
FCastExpr::~FCastExpr() {}
//----------------------------------------------------------------------------
RTTI::FAtom FCastExpr::Eval(FParseContext* context) const {
    Assert(context);

    RTTI::FAtom atom = _expr->Eval(context);

    if (atom) {
        const RTTI::PTypeTraits dst = RTTI::MakeTraits(_typeId);

        if (dst != atom.Traits()) {
            RTTI::FAtom casted = context->CreateAtom(dst);
            if (false == atom.PromoteMove(casted))
                PPE_THROW_IT(FParserException("invalid cast", this));

            atom = casted;
        }
    }

    return atom;
}
//----------------------------------------------------------------------------
FString FCastExpr::ToString() const {
    FStringBuilder oss;
    oss << RTTI::MakeTraits(_typeId)->TypeInfos().Name() << ": ";
    oss << _expr->ToString();
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
