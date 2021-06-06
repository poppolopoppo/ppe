#include "stdafx.h"

#include "RTTI/AtomHeap.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAtomHeap::FAtomHeap()
{}
//----------------------------------------------------------------------------
FAtomHeap::~FAtomHeap() {
    ReleaseAll();
    Assert(nullptr == _destructibles);
}
//----------------------------------------------------------------------------
NO_INLINE void FAtomHeap::DiscardAll() {
    THIS_THREADRESOURCE_CHECKACCESS();

    ReleaseDestructibles_();

    _heap.DiscardAll();
}
//----------------------------------------------------------------------------
NO_INLINE void FAtomHeap::ReleaseAll() {
    THIS_THREADRESOURCE_CHECKACCESS();

    ReleaseDestructibles_();

    _heap.ReleaseAll();
}
//----------------------------------------------------------------------------
FAtom FAtomHeap::MakeAtomUinitialized_(const PTypeTraits& traits) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const FTypeInfos typeInfos = traits->TypeInfos();
    Assert(typeInfos.SizeInBytes());

    if (typeInfos.Flags() & ETypeFlags::TriviallyDestructible)
        return { _heap.Allocate(typeInfos.SizeInBytes()), traits };

    // track non-trivial type instances for destruction
    FPendingDestroy_* const pBlock = INPLACE_NEW(
        _heap.Allocate(sizeof(FPendingDestroy_) + typeInfos.SizeInBytes()),
        FPendingDestroy_ );

    pBlock->Traits = traits;
    pBlock->pNext = _destructibles;
    _destructibles = pBlock;

    return { pBlock + 1, traits };
}
//----------------------------------------------------------------------------
void FAtomHeap::ReleaseDestructibles_() {
    // destroy non-trivial type instances :
    for (FPendingDestroy_* it = _destructibles; it; ) {
        FPendingDestroy_* const pNext = it->pNext;

        it->Traits->Destroy(it + 1);
        _heap.Deallocate(it, sizeof(FPendingDestroy_) + it->Traits->SizeInBytes());

        it = pNext;
    }

    _destructibles = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
