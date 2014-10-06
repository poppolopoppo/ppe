 #pragma once

#include "Core.h"
#include "HashMap.h"
#include "RefPtr.h"

namespace Core {
namespace RTTI {
    FWD_REFPTR(MetaAtom);
    FWD_REFPTR(MetaObject);
    FWD_REFPTR(MetaTransaction);
    class MetaObjectName;
}}

namespace Core {
namespace Parser {
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

    void AddLocal(const RTTI::MetaObjectName& name, RTTI::MetaAtom *value);
    void RemoveLocal(const RTTI::MetaObjectName& name, RTTI::MetaAtom *value);

    RTTI::MetaAtom *GetGlobal(const RTTI::MetaObjectName& name) const;

    void AddGlobal(const RTTI::MetaObjectName& name, RTTI::MetaAtom *value);
    void RemoveGlobal(const RTTI::MetaObjectName& name, RTTI::MetaAtom *value);

    RTTI::MetaAtom *GetAny(const RTTI::MetaObjectName& name) const;

private:
    const ParseContext *_parent;
    RTTI::PMetaObject _scopeObject;
    RTTI::PMetaTransaction _transaction;
    HASHMAP_THREAD_LOCAL(Parser, RTTI::MetaObjectName, RTTI::PMetaAtom) _localScope;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
