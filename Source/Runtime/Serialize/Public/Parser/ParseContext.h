 #pragma once

#include "Serialize.h"

#include "RTTI_fwd.h"
#include "RTTI/TypeTraits.h"

#include "Container/HashMap.h"
#include "Memory/RefPtr.h"

namespace PPE {
class FLinearHeap;
namespace Parser {
FWD_REFPTR(ParseExpression);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FParseContext {
public:
    explicit FParseContext(Meta::FForceInit);
    explicit FParseContext(const RTTI::PAtomHeap& atomHeap);
    explicit FParseContext(const FParseContext* parent);

    virtual ~FParseContext();

    const FParseContext* Parent() const { return _parent; }
    const FParseContext* GlobalScope() const;

    RTTI::FMetaObject* ScopeObject() const { return _scopeObject.get(); }
    void SetScopeObject(RTTI::FMetaObject *object);

    RTTI::FAtom GetLocal(const RTTI::FName& name) const;

    void AddLocal(const FParseExpression* expr, const RTTI::FName& name, const RTTI::FAtom& value);
    void RemoveLocal(const FParseExpression* expr, const RTTI::FName& name, const RTTI::FAtom& value);

    RTTI::FAtom GetGlobal(const RTTI::FName& name) const;

    void AddGlobal(const FParseExpression* expr, const RTTI::FName& name, const RTTI::FAtom& value);
    void RemoveGlobal(const FParseExpression* expr, const RTTI::FName& name, const RTTI::FAtom& value);

    RTTI::FAtom GetAny(const RTTI::FName& name) const;

    RTTI::FAtom CreateAtom(const RTTI::PTypeTraits& traits);
    RTTI::FAtom CreateAtom(const RTTI::PTypeTraits& traits, void* rvalue);

    template <typename T>
    RTTI::FAtom CreateAtomFrom(T&& rvalue) {
        return CreateAtom(RTTI::MakeTraits<T>(), &rvalue);
    }

    template <typename _Container>
    typename _Container::allocator_type CreateHeapContainer() {
        return typename _Container::allocator_type(LinearHeap_());
    }

private:
    const FParseContext* const _parent;
    const RTTI::PAtomHeap _atomHeap;

    RTTI::PMetaObject _scopeObject;
    HASHMAP(Parser, RTTI::FName, RTTI::FAtom) _localScope;

    FLinearHeap& LinearHeap_() const;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
