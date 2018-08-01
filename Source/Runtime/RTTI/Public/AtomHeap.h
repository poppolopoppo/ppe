#pragma once

#include "RTTI.h"

#include "Atom.h"
#include "NativeTypes.h"

#include "Allocator/LinearHeap.h"
#include "Allocator/LinearHeapAllocator.h"
#include "Container/Vector.h"
#include "Memory/RefPtr.h"
#include "Meta/ThreadResource.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use a FLinearHeap for transient FAtom allocation
//----------------------------------------------------------------------------
FWD_REFPTR(AtomHeap);
class PPE_RTTI_API FAtomHeap : public FRefCountable, Meta::FThreadResource {
public:
    FAtomHeap();
    ~FAtomHeap();

    FAtomHeap(const FAtomHeap&) = delete;
    FAtomHeap& operator =(const FAtomHeap&) = delete;

    FLinearHeap& Heap() { return _heap; }

    FAtom Allocate(ENativeType type) {
        return Allocate(MakeTraits(type));
    }

    FAtom Allocate(const PTypeTraits& traits) {
        FAtom atom = MakeAtomUinitialized_(traits);
        traits->Construct(atom.Data());
        return atom;
    }

    FAtom AllocateCopy(const PTypeTraits& traits, const void* other) {
        FAtom atom = MakeAtomUinitialized_(traits);
        traits->ConstructCopy(atom.Data(), other);
        return atom;
    }

    FAtom AllocateMove(const PTypeTraits& traits, void* rvalue) {
        FAtom atom = MakeAtomUinitialized_(traits);
        traits->ConstructMove(atom.Data(), rvalue);
        return atom;
    }

    template <typename T>
    FAtom AllocateCopy(const T& other) {
        return AllocateMove(MakeTraits<T>(), &other);
    }

    template <typename T>
    FAtom AllocateMove(T&& rvalue) {
        return AllocateMove(MakeTraits<T>(), &rvalue);
    }

    void ReleaseAll();

private:
    LINEARHEAP(Atom) _heap;
    VECTOR(Atom, FAtom) _destructibles;

    FAtom MakeAtomUinitialized_(const PTypeTraits& traits);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
