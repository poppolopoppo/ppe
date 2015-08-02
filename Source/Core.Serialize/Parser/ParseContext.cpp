#include "stdafx.h"

#include "ParseContext.h"

#include "Core.RTTI/Atom/MetaAtom.h"
#include "Core.RTTI/Atom/MetaAtomDatabase.h"
#include "Core.RTTI/Class/MetaClass.h"
#include "Core.RTTI/Class/MetaClassName.h"
#include "Core.RTTI/MetaTransaction.h"
#include "Core.RTTI/Object/MetaObject.h"
#include "Core.RTTI/Object/MetaObjectName.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ParseContext::ParseContext(RTTI::MetaTransaction *transaction, const ParseContext *parent /* = nullptr */)
:   _parent(parent), _transaction(transaction){
    _localScope.reserve(32);
}
//----------------------------------------------------------------------------
ParseContext::~ParseContext() {}
//----------------------------------------------------------------------------
void ParseContext::SetScopeObject(RTTI::MetaObject *object) {
    Assert(object != _scopeObject);
    _scopeObject = object;
}
//----------------------------------------------------------------------------
RTTI::MetaAtom *ParseContext::GetLocal(const RTTI::MetaObjectName& name) const {
    Assert(!name.empty());

    const auto it = _localScope.find(name);

    return (_localScope.end() == it) ? nullptr : it->second.get();
}
//----------------------------------------------------------------------------
void ParseContext::AddLocal(const RTTI::MetaObjectName& name, RTTI::MetaAtom *value) {
    Assert(!name.empty());
    Assert(value);

    RTTI::PMetaAtom& local = _localScope[name];

    local = value;
}
//----------------------------------------------------------------------------
void ParseContext::RemoveLocal(const RTTI::MetaObjectName& name, RTTI::MetaAtom *value) {
    Assert(!name.empty());
    Assert(value);

    const auto it = _localScope.find(name);
    Assert(_localScope.end() != it);
    Assert(it->second == value);

    _localScope.erase(it);
}
//----------------------------------------------------------------------------
void ParseContext::AddGlobal(const RTTI::MetaObjectName& name, RTTI::MetaAtom *value) {
    Assert(!name.empty());
    Assert(value);

    _transaction->Export(name, value, true);
}
//----------------------------------------------------------------------------
void ParseContext::RemoveGlobal(const RTTI::MetaObjectName& name, RTTI::MetaAtom *value) {
    Assert(!name.empty());
    Assert(value);

    _transaction->Remove(name, value);
}
//----------------------------------------------------------------------------
RTTI::MetaAtom *ParseContext::GetGlobal(const RTTI::MetaObjectName& name) const {
    Assert(!name.empty());

    return RTTI::MetaAtomDatabase::Instance().GetIFP(name);
}
//----------------------------------------------------------------------------
RTTI::MetaAtom *ParseContext::GetAny(const RTTI::MetaObjectName& name) const {
    const ParseContext *ctx = this;
    do {
        RTTI::MetaAtom *result;

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
