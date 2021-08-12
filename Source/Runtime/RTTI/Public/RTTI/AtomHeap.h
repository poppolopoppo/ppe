#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Atom.h"
#include "RTTI/NativeTypes.h"

#include "Allocator/SlabHeap.h"
#include "Allocator/SlabAllocator.h"
#include "Memory/RefPtr.h"
#include "Thread/DataRaceCheck.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Use a FSlabHeap for transient FAtom allocation
//----------------------------------------------------------------------------
class PPE_RTTI_API FAtomHeap : public FRefCountable, FDataRaceCheckResource {
public:
    FAtomHeap();
    ~FAtomHeap();

    FAtomHeap(const FAtomHeap&) = delete;
    FAtomHeap& operator =(const FAtomHeap&) = delete;

    SLABHEAP_POOLED(Atom)& Heap() { return _heap; }

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

    void DiscardAll();
    void ReleaseAll();

private:
    struct ALIGN(ALLOCATION_BOUNDARY) FPendingDestroy_ {
        PTypeTraits Traits;
        FPendingDestroy_* pNext;
    };

    SLABHEAP_POOLED(Atom) _heap;
    FPendingDestroy_* _destructibles{ nullptr };

    FAtom MakeAtomUinitialized_(const PTypeTraits& traits);
    void ReleaseDestructibles_();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
