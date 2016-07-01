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
ParseContext::ParseContext(const ParseContext *parent /* = nullptr */)
:   _parent(parent) {
    _localScope.reserve(8);
}
//----------------------------------------------------------------------------
ParseContext::~ParseContext() {}
//----------------------------------------------------------------------------
void ParseContext::SetScopeObject(RTTI::MetaObject *object) {
    Assert(object != _scopeObject);
    _scopeObject = object;
}
//----------------------------------------------------------------------------
RTTI::MetaAtom *ParseContext::GetLocal(const RTTI::Name& name) const {
    Assert(!name.empty());

    const auto it = _localScope.find(name);

    return (_localScope.end() == it) ? nullptr : it->second.get();
}
//----------------------------------------------------------------------------
void ParseContext::AddLocal(const ParseExpression* expr, const RTTI::Name& name, RTTI::MetaAtom *value) {
    Assert(!name.empty());
    Assert(value);

    RTTI::PMetaAtom& local = _localScope[name];

    if (local)
        throw ParserException("failed to overwrite local variable", expr);

    local = value;
}
//----------------------------------------------------------------------------
void ParseContext::RemoveLocal(const ParseExpression* expr, const RTTI::Name& name, RTTI::MetaAtom *value) {
    Assert(!name.empty());
    Assert(value);

    const auto it = _localScope.find(name);

    if (_localScope.end() == it)
        throw ParserException("failed to remove unknown local variable", expr);

    if (it->second != value)
        throw ParserException("failed to remove local variable with wrong value", expr);

    _localScope.erase(it);
}
//----------------------------------------------------------------------------
void ParseContext::AddGlobal(const ParseExpression* expr, const RTTI::Name& name, RTTI::MetaAtom* value) {
    Assert(!name.empty());
    Assert(value);

    const ParseContext* ctx = this;
    while (ctx->_parent)
        ctx = ctx->_parent;
    Assert(ctx);

    RTTI::PMetaAtom& global = ctx->_globalScope[name];

    if (global)
        throw ParserException("failed to overwrite global variable", expr);

    const auto* atom = value->As<RTTI::PMetaObject>();

    if (nullptr == atom)
        throw ParserException("exported atom is not an object", expr);

    global = value;
    const RTTI::PMetaObject& obj = atom->Wrapper();
    if (obj)
        obj->RTTI_Export(name);
}
//----------------------------------------------------------------------------
void ParseContext::RemoveGlobal(const ParseExpression* expr, const RTTI::Name& name, RTTI::MetaAtom* value) {
    Assert(!name.empty());
    Assert(value);

    const ParseContext* ctx = this;
    while (ctx->_parent)
        ctx = ctx->_parent;
    Assert(ctx);

    const auto it = ctx->_globalScope.find(name);

    if (ctx->_globalScope.end() == it)
        throw ParserException("failed to remove unknown global variable", expr);

    if (it->second != value)
        throw ParserException("failed to remove global variable with wrong value", expr);

    const auto* atom = it->second->As<RTTI::PMetaObject>();

    if (nullptr == atom)
        throw ParserException("exported atom is not an object", expr);

    RTTI::PMetaObject obj = atom->Wrapper();

    ctx->_globalScope.erase(it);

    if (obj)
        obj->RTTI_Unexport();
}
//----------------------------------------------------------------------------
RTTI::MetaAtom *ParseContext::GetGlobal(const RTTI::Name& name) const {
    Assert(!name.empty());

    const ParseContext* ctx = this;
    while (ctx->_parent)
        ctx = ctx->_parent;
    Assert(ctx);

    const auto it = ctx->_globalScope.find(name);

    return (ctx->_globalScope.end() == it) ? nullptr : it->second.get();
}
//----------------------------------------------------------------------------
RTTI::MetaAtom *ParseContext::GetAny(const RTTI::Name& name) const {
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
