 #pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace RTTI {
    FWD_REFPTR(MetaAtom);
    FWD_REFPTR(MetaObject);
    FWD_REFPTR(MetaTransaction);
    class Name;
}}

namespace Core {
namespace Parser {
FWD_REFPTR(ParseExpression);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParseContext {
public:
    explicit ParseContext(const ParseContext *parent = nullptr);
    virtual ~ParseContext();

    const ParseContext *Parent() const { return _parent; }

    RTTI::MetaObject *ScopeObject() const { return _scopeObject.get(); }
    void SetScopeObject(RTTI::MetaObject *object);

    RTTI::MetaAtom *GetLocal(const RTTI::Name& name) const;

    void AddLocal(const ParseExpression* expr, const RTTI::Name& name, RTTI::MetaAtom *value);
    void RemoveLocal(const ParseExpression* expr, const RTTI::Name& name, RTTI::MetaAtom *value);

    RTTI::MetaAtom *GetGlobal(const RTTI::Name& name) const;

    void AddGlobal(const ParseExpression* expr, const RTTI::Name& name, RTTI::MetaAtom *value);
    void RemoveGlobal(const ParseExpression* expr, const RTTI::Name& name, RTTI::MetaAtom *value);

    RTTI::MetaAtom *GetAny(const RTTI::Name& name) const;

private:
    const ParseContext *_parent;
    RTTI::PMetaObject _scopeObject;
    HASHMAP_THREAD_LOCAL(Parser, RTTI::Name, RTTI::PMetaAtom) _localScope;
    mutable HASHMAP_THREAD_LOCAL(Parser, RTTI::Name, RTTI::PMetaAtom) _globalScope;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
