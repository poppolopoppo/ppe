 #pragma once

#include "Serialize_fwd.h"

#include "RTTI_fwd.h"
#include "Allocator/SlabHeap.h"
#include "RTTI/Atom.h"
#include "RTTI/TypeTraits.h"

#include "Container/HashMap.h"
#include "Memory/RefPtr.h"
#include "IO/StringBuilder.h"

namespace PPE {
namespace Parser {
FWD_REFPTR(ParseExpression);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FParseContext : Meta::FNonCopyableNorMovable {
public:
    using local_scope_t = HASHMAP(Parser, RTTI::FName, RTTI::FAtom);

    explicit FParseContext(Meta::FForceInit);
    explicit FParseContext(const RTTI::PAtomHeap& atomHeap);
    explicit FParseContext(const FParseContext* parent);

    ~FParseContext();

    const FParseContext* Parent() const { return _parent; }
    const FParseContext* GlobalScope() const;
    const local_scope_t& LocalScope() const { return _localScope; }

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
    RTTI::FAtom CreateAtomCopy(const RTTI::FAtom& atom);

    template <typename T>
    RTTI::FAtom CreateAtomFrom(T&& rvalue) {
        return CreateAtom(RTTI::MakeTraits<T>(), &rvalue);
    }

    template <typename _Container>
    typename _Container::allocator_type CreateHeapContainer() {
        return typename _Container::allocator_type(Heap_());
    }

private:
    const FParseContext* const _parent;
    const RTTI::PAtomHeap _atomHeap;

    RTTI::PMetaObject _scopeObject;
    local_scope_t _localScope;

    SLABHEAP(Atom)& Heap_() const;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
