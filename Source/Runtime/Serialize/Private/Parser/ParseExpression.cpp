#include "stdafx.h"

#include "Parser/ParseExpression.h"

#include "Parser/Parser.h"
#include "Parser/ParseStatement.h"

#include "RTTI/Any.h"
#include "RTTI/AtomHeap.h"
#include "MetaClass.h"
#include "MetaEnum.h"
#include "MetaDatabase.h"
#include "MetaFunction.h"
#include "MetaObject.h"
#include "MetaProperty.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FParseExpression::FParseExpression(const Lexer::FSpan& site)
:   FParseItem(site) {}
//----------------------------------------------------------------------------
FParseExpression::~FParseExpression() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVariableExport::FVariableExport(const RTTI::FName& name, const PCParseExpression& value, const EFlags scope, const Lexer::FSpan& site)
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
FVariableReference::FVariableReference(const RTTI::FPathName& pathName, const Lexer::FSpan& site)
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
        const RTTI::FAtom local = context->GetAny(_pathName.Identifier);

        if (not local)
            PPE_THROW_IT(FParserException("unknown variable name", this));

        value = context->CreateAtom(local.Traits());
        local.Copy(value);
    }
    else { // global :
        // #TODO use linker instead for MT ???
        const RTTI::FMetaDatabaseReadable metaDB;
        RTTI::PMetaObject objIFP{ metaDB->ObjectIFP(_pathName) };

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
FObjectDefinition::FObjectDefinition(const RTTI::FName& name, const Lexer::FSpan& site)
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

    // #TODO use linker instead for MT ???
    const RTTI::FMetaDatabaseReadable metaDB;
    const RTTI::FMetaClass *metaclass = metaDB->ClassIFP(_name);
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
FPropertyReference::FPropertyReference(
    const PCParseExpression& object,
    const RTTI::FName& member,
    const Lexer::FSpan& site)
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
FTupleExpr::FTupleExpr(const Lexer::FSpan& site)
:   FParseExpression(site)
{}
//----------------------------------------------------------------------------
FTupleExpr::FTupleExpr(const TMemoryView<PCParseExpression>& elts, const Lexer::FSpan& site)
:   FParseExpression(site) {
    _elements.insert(_elements.end(),
        MakeMoveIterator(elts.begin()),
        MakeMoveIterator(elts.end()) );
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
FArrayExpr::FArrayExpr(const Lexer::FSpan& site)
:   FParseExpression(site) {}
//----------------------------------------------------------------------------
FArrayExpr::FArrayExpr(const TMemoryView<PCParseExpression>& elts, const Lexer::FSpan& site)
:   FParseExpression(site) {
    _items.insert(_items.end(),
        MakeMoveIterator(elts.begin()),
        MakeMoveIterator(elts.end()));
}
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
FDictionaryExpr::FDictionaryExpr(const Lexer::FSpan& site)
:   FParseExpression(site) {}
//----------------------------------------------------------------------------
FDictionaryExpr::FDictionaryExpr(dico_type&& rdico, const Lexer::FSpan& site)
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
FCastExpr::FCastExpr(const RTTI::PTypeTraits& traits, const FParseExpression* expr, const Lexer::FSpan& site)
:   FParseExpression(site)
,   _traits(traits)
,   _expr(expr) {
    Assert(traits);
    Assert(expr);
}
//----------------------------------------------------------------------------
FCastExpr::~FCastExpr()
{}
//----------------------------------------------------------------------------
RTTI::FAtom FCastExpr::Eval(FParseContext* context) const {
    Assert(context);

    RTTI::FAtom atom = _expr->Eval(context);

    if (atom && _traits != atom.Traits()) {
        RTTI::FAtom casted = context->CreateAtom(_traits);

        if (false == atom.PromoteMove(casted))
            PPE_THROW_IT(FParserException("invalid cast", this));

        return casted;
    }
    else {
        return atom;
    }
}
//----------------------------------------------------------------------------
FString FCastExpr::ToString() const {
    FStringBuilder oss;
    oss << _traits->TypeInfos().Name() << ": " << _expr->ToString();
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFunctionCall::FFunctionCall(PCParseExpression&& obj, const RTTI::FName& funcname, const TMemoryView<const PCParseExpression>& args, const Lexer::FSpan& site)
:   FParseExpression(site)
,   _obj(std::move(obj))
,   _funcname(funcname)
,   _args(args) {
    Assert_NoAssume(_obj);
    Assert_NoAssume(not _funcname.empty());
}
//----------------------------------------------------------------------------
FFunctionCall::~FFunctionCall() {}
//----------------------------------------------------------------------------
RTTI::FAtom FFunctionCall::Eval(FParseContext* context) const {
    Assert(context);

    RTTI::FAtom atom = _obj->Eval(context);

    if (not atom)
        PPE_THROW_IT(FParserException("void object reference in function call", _obj->Site(), this));

    const RTTI::PMetaObject* const objIFP = atom.TypedConstDataIFP<RTTI::PMetaObject>();

    if (not objIFP)
        PPE_THROW_IT(FParserException("can only call functions on objects", _obj->Site(), this));
    if (not *objIFP)
        PPE_THROW_IT(FParserException("null object reference in function call", _obj->Site(), this));

    const RTTI::PMetaObject obj{ *objIFP };

    const RTTI::FMetaClass* const klass = obj->RTTI_Class();
    Assert(klass);

    const RTTI::FMetaFunction* const funcIFP = klass->FunctionIFP(_funcname, RTTI::EFunctionFlags::Public);
    if (not funcIFP) {
        if (klass->FunctionIFP(_funcname, RTTI::EFunctionFlags::All))
            PPE_THROW_IT(FParserException("can't call non public function", this));
        else
            PPE_THROW_IT(FParserException("unknown function name", this));
    }

    const TMemoryView<const RTTI::FMetaParameter> prms = funcIFP->Parameters();

    // #TODO : handle optional parameters
    if (prms.size() < _args.size())
        PPE_THROW_IT(FParserException("not enough parameters given to function call", this));
    else if (prms.size() > _args.size())
        PPE_THROW_IT(FParserException("too much parameters given to function call", this));
    Assert(prms.size() == _args.size());

    using atoms_t = VECTOR_LINEARHEAP(RTTI::FAtom);
    atoms_t evalArgs{ context->CreateHeapContainer<atoms_t>() };
    evalArgs.reserve(_args.size());

    forrange(i, 0, _args.size()) {
        RTTI::FAtom v = _args[i]->Eval(context);

        if (v.Traits() != prms[i].Traits()) {
            // not matching : try promotion

            RTTI::FAtom w = context->CreateAtom(prms[i].Traits());
            if (v.PromoteMove(w))
                evalArgs.emplace_back(std::move(w)); // promotion succeeded :)
            else
                PPE_THROW_IT(FParserException("invalid parameter type given to function call", _args[i]->Site(), this));
        }
        else {
            // correct type : push as-is
            evalArgs.emplace_back(std::move(v));
        }
    }

    RTTI::FAtom result;
    if (funcIFP->HasReturnValue())
        result = context->CreateAtom(funcIFP->Result());

    funcIFP->Invoke(*obj, result, evalArgs.MakeConstView());

    return result;
}
//----------------------------------------------------------------------------
FString FFunctionCall::ToString() const {
    FStringBuilder oss;
    oss << _obj->ToString()
        << '.' << _funcname << '(';

    auto sep = Fmt::NotFirstTime(MakeStringView(", "));
    for (const PCParseExpression& e : _args)
        oss << sep << e->ToString();

    oss << ')';
    return oss.ToString();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
