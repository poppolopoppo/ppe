#include "stdafx.h"

#include "Parser/ParseContext.h"

#include "RTTI/Atom.h"
#include "RTTI/AtomHeap.h"
#include "RTTI/Typedefs.h"
#include "RTTI/TypeTraits.h"
#include "MetaClass.h"
#include "MetaDatabase.h"
#include "MetaObject.h"
#include "MetaTransaction.h"

#include "Parser/Parser.h"
#include "Parser/ParseExpression.h"
#include "Parser/ParseStatement.h"

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FParseContext::FParseContext(Meta::FForceInit)
:   FParseContext(NEW_REF(Parser, RTTI::FAtomHeap)())
{}
//----------------------------------------------------------------------------
FParseContext::FParseContext(const RTTI::PAtomHeap& atomHeap)
:   _parent(nullptr)
,   _atomHeap(atomHeap) {
    Assert(_atomHeap);
}
//----------------------------------------------------------------------------
FParseContext::FParseContext(const FParseContext *parent /* = nullptr */)
:   _parent(parent)
,   _atomHeap(_parent->_atomHeap) {
    Assert(_atomHeap);
}
//----------------------------------------------------------------------------
FParseContext::~FParseContext() {}
//----------------------------------------------------------------------------
const FParseContext* FParseContext::GlobalScope() const {
    const FParseContext* ctx = this;
    while (ctx->_parent)
        ctx = ctx->_parent;

    Assert(ctx);
    return ctx;
}
//----------------------------------------------------------------------------
void FParseContext::SetScopeObject(RTTI::FMetaObject *object) {
    Assert(object != _scopeObject);
    _scopeObject = object;
}
//----------------------------------------------------------------------------
RTTI::FAtom FParseContext::GetLocal(const RTTI::FName& name) const {
    Assert(!name.empty());

    RTTI::FAtom result;
    const auto it = _localScope.find(name);
    if (_localScope.end() != it) {
        result = it->second;
        Assert(result);
    }
    return result;
}
//----------------------------------------------------------------------------
void FParseContext::AddLocal(const FParseExpression* expr, const RTTI::FName& name, const RTTI::FAtom& value) {
    Assert(!name.empty());

    if (not value)
        PPE_THROW_IT(FParserException("can't assign void to a variable", expr));

    if (not _localScope.try_emplace(name, value).second)
        PPE_THROW_IT(FParserException("failed to overwrite local variable", expr));
}
//----------------------------------------------------------------------------
void FParseContext::RemoveLocal(const FParseExpression* expr, const RTTI::FName& name, const RTTI::FAtom& value) {
    Assert(!name.empty());
    Assert(value);

    const auto it = _localScope.find(name);

    if (_localScope.end() == it)
        PPE_THROW_IT(FParserException("failed to remove unknown local variable", expr));

    if (it->second != value)
        PPE_THROW_IT(FParserException("failed to remove local variable with wrong value", expr));

    _localScope.erase(it);
}
//----------------------------------------------------------------------------
void FParseContext::AddGlobal(const FParseExpression* expr, const RTTI::FName& name, const RTTI::FAtom& value) {
    Assert(!name.empty());
    Assert(value);

    auto* gbl = const_cast<FParseContext*>(GlobalScope());
    if (not gbl->_localScope.try_emplace(name, value).second)
        PPE_THROW_IT(FParserException("failed to overwrite global variable", expr));

    const auto* atom = value.TypedConstDataIFP<RTTI::PMetaObject>();
    if (nullptr == atom)
        PPE_THROW_IT(FParserException("exported atom is not an object", expr));
}
//----------------------------------------------------------------------------
void FParseContext::RemoveGlobal(const FParseExpression* expr, const RTTI::FName& name, const RTTI::FAtom& value) {
    Assert(!name.empty());
    Assert(value);

    auto* gbl = const_cast<FParseContext*>(GlobalScope());
    const auto it = gbl->_localScope.find(name);
    if (gbl->_localScope.end() == it)
        PPE_THROW_IT(FParserException("failed to remove unknown global variable", expr));

    if (it->second != value)
        PPE_THROW_IT(FParserException("failed to remove global variable with wrong value", expr));

    gbl->_localScope.erase(it);
}
//----------------------------------------------------------------------------
RTTI::FAtom FParseContext::GetGlobal(const RTTI::FName& name) const {
    return GlobalScope()->GetLocal(name);
}
//----------------------------------------------------------------------------
RTTI::FAtom FParseContext::GetAny(const RTTI::FName& name) const {
    RTTI::FAtom result;
    const FParseContext *ctx = this;
    do {
        result = ctx->GetLocal(name);
        if (result)
            break;

        ctx = ctx->_parent;
    } while (ctx);
    return result;
}
//----------------------------------------------------------------------------
RTTI::FAtom FParseContext::CreateAtom(const RTTI::PTypeTraits& traits) {
    return _atomHeap->Allocate(traits);
}
//----------------------------------------------------------------------------
RTTI::FAtom FParseContext::CreateAtom(const RTTI::PTypeTraits& traits, void* rvalue) {
    return _atomHeap->AllocateMove(traits, rvalue);
}
//----------------------------------------------------------------------------
FPooledLinearHeap& FParseContext::LinearHeap_() const {
    return _atomHeap->Heap();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
