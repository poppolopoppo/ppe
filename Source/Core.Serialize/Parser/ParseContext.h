 #pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
    FWD_REFPTR(MetaAtom);
    FWD_REFPTR(MetaObject);
    FWD_REFPTR(MetaTransaction);
    class FName;
}}

namespace Core {
namespace Parser {
FWD_REFPTR(ParseExpression);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParseContext {
public:
    explicit FParseContext(const FParseContext *parent = nullptr);
    virtual ~FParseContext();

    const FParseContext *Parent() const { return _parent; }

    RTTI::FMetaObject *ScopeObject() const { return _scopeObject.get(); }
    void SetScopeObject(RTTI::FMetaObject *object);

    RTTI::FMetaAtom *GetLocal(const RTTI::FName& name) const;

    void AddLocal(const FParseExpression* expr, const RTTI::FName& name, RTTI::FMetaAtom *value);
    void RemoveLocal(const FParseExpression* expr, const RTTI::FName& name, RTTI::FMetaAtom *value);

    RTTI::FMetaAtom *GetGlobal(const RTTI::FName& name) const;

    void AddGlobal(const FParseExpression* expr, const RTTI::FName& name, RTTI::FMetaAtom *value);
    void RemoveGlobal(const FParseExpression* expr, const RTTI::FName& name, RTTI::FMetaAtom *value);

    RTTI::FMetaAtom *GetAny(const RTTI::FName& name) const;

private:
    const FParseContext *_parent;
    RTTI::PMetaObject _scopeObject;
    HASHMAP_THREAD_LOCAL(Parser, RTTI::FName, RTTI::PMetaAtom) _localScope;
    mutable HASHMAP_THREAD_LOCAL(Parser, RTTI::FName, RTTI::PMetaAtom) _globalScope;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
