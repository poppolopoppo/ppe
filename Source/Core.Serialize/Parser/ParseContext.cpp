#include "stdafx.h"

#include "ParseContext.h"

#include "Parser.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaAtomDatabase.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaTransaction.h"
#include "Core.RTTI/Typedefs.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FParseContext::FParseContext(const FParseContext *parent /* = nullptr */)
:   _parent(parent) {
    _localScope.reserve(8);
}
//----------------------------------------------------------------------------
FParseContext::~FParseContext() {}
//----------------------------------------------------------------------------
void FParseContext::SetScopeObject(RTTI::FMetaObject *object) {
    Assert(object != _scopeObject);
    _scopeObject = object;
}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *FParseContext::GetLocal(const RTTI::FName& name) const {
    Assert(!name.empty());

    const auto it = _localScope.find(name);

    return (_localScope.end() == it) ? nullptr : it->second.get();
}
//----------------------------------------------------------------------------
void FParseContext::AddLocal(const FParseExpression* expr, const RTTI::FName& name, RTTI::FMetaAtom *value) {
    Assert(!name.empty());
    Assert(value);

    RTTI::PMetaAtom& local = _localScope[name];

    if (local)
        CORE_THROW_IT(FParserException("failed to overwrite local variable", expr));

    local = value;
}
//----------------------------------------------------------------------------
void FParseContext::RemoveLocal(const FParseExpression* expr, const RTTI::FName& name, RTTI::FMetaAtom *value) {
    Assert(!name.empty());
    Assert(value);

    const auto it = _localScope.find(name);

    if (_localScope.end() == it)
        CORE_THROW_IT(FParserException("failed to remove unknown local variable", expr));

    if (it->second != value)
        CORE_THROW_IT(FParserException("failed to remove local variable with wrong value", expr));

    _localScope.erase(it);
}
//----------------------------------------------------------------------------
void FParseContext::AddGlobal(const FParseExpression* expr, const RTTI::FName& name, RTTI::FMetaAtom* value) {
    Assert(!name.empty());
    Assert(value);

    const FParseContext* ctx = this;
    while (ctx->_parent)
        ctx = ctx->_parent;
    Assert(ctx);

    RTTI::PMetaAtom& global = ctx->_globalScope[name];

    if (global)
        CORE_THROW_IT(FParserException("failed to overwrite global variable", expr));

    const auto* atom = value->As<RTTI::PMetaObject>();

    if (nullptr == atom)
        CORE_THROW_IT(FParserException("exported atom is not an object", expr));

    global = value;
    const RTTI::PMetaObject& obj = atom->Wrapper();
    if (obj)
        obj->RTTI_Export(name);
}
//----------------------------------------------------------------------------
void FParseContext::RemoveGlobal(const FParseExpression* expr, const RTTI::FName& name, RTTI::FMetaAtom* value) {
    Assert(!name.empty());
    Assert(value);

    const FParseContext* ctx = this;
    while (ctx->_parent)
        ctx = ctx->_parent;
    Assert(ctx);

    const auto it = ctx->_globalScope.find(name);

    if (ctx->_globalScope.end() == it)
        CORE_THROW_IT(FParserException("failed to remove unknown global variable", expr));

    if (it->second != value)
        CORE_THROW_IT(FParserException("failed to remove global variable with wrong value", expr));

    const auto* atom = it->second->As<RTTI::PMetaObject>();

    if (nullptr == atom)
        CORE_THROW_IT(FParserException("exported atom is not an object", expr));

    RTTI::PMetaObject obj = atom->Wrapper();

    ctx->_globalScope.erase(it);

    if (obj)
        obj->RTTI_Unexport();
}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *FParseContext::GetGlobal(const RTTI::FName& name) const {
    Assert(!name.empty());

    const FParseContext* ctx = this;
    while (ctx->_parent)
        ctx = ctx->_parent;
    Assert(ctx);

    const auto it = ctx->_globalScope.find(name);

    return (ctx->_globalScope.end() == it) ? nullptr : it->second.get();
}
//----------------------------------------------------------------------------
RTTI::FMetaAtom *FParseContext::GetAny(const RTTI::FName& name) const {
    const FParseContext *ctx = this;
    do {
        RTTI::FMetaAtom *result;

        if (nullptr != (result = ctx->GetLocal(name)))
            return result;

        ctx = ctx->_parent;
    } while (ctx);

    return GetGlobal(name);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
