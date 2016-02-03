 #pragma once

#include "Core/Core.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
    FWD_REFPTR(MetaAtom);
    FWD_REFPTR(MetaObject);
    FWD_REFPTR(MetaTransaction);
    class MetaObjectName;
}}

namespace Core {
namespace Parser {
FWD_REFPTR(ParseExpression);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParseContext {
public:
    explicit ParseContext(RTTI::MetaTransaction *transaction, const ParseContext *parent = nullptr);
    virtual ~ParseContext();

    const ParseContext *Parent() const { return _parent; }

    RTTI::MetaObject *ScopeObject() const { return _scopeObject.get(); }
    void SetScopeObject(RTTI::MetaObject *object);

    RTTI::MetaTransaction *Transaction() const { return _transaction.get(); }

    RTTI::MetaAtom *GetLocal(const RTTI::MetaObjectName& name) const;

    void AddLocal(const ParseExpression* expr, const RTTI::MetaObjectName& name, RTTI::MetaAtom *value);
    void RemoveLocal(const ParseExpression* expr, const RTTI::MetaObjectName& name, RTTI::MetaAtom *value);

    RTTI::MetaAtom *GetGlobal(const RTTI::MetaObjectName& name) const;

    void AddGlobal(const ParseExpression* expr, const RTTI::MetaObjectName& name, RTTI::MetaAtom *value);
    void RemoveGlobal(const ParseExpression* expr, const RTTI::MetaObjectName& name, RTTI::MetaAtom *value);

    RTTI::MetaAtom *GetAny(const RTTI::MetaObjectName& name) const;

private:
    const ParseContext *_parent;
    RTTI::PMetaObject _scopeObject;
    RTTI::PMetaTransaction _transaction;
    HASHMAP_THREAD_LOCAL(Parser, RTTI::MetaObjectName, RTTI::PMetaAtom) _localScope;
    mutable HASHMAP_THREAD_LOCAL(Parser, RTTI::MetaObjectName, RTTI::PMetaAtom) _globalScope;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
