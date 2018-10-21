#include "stdafx.h"

#include "RTTI/AtomHeap.h"

#include "MetaObject.h"

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
    Assert(_destructibles.empty());
}
//----------------------------------------------------------------------------
NO_INLINE void FAtomHeap::ReleaseAll() {
    THIS_THREADRESOURCE_CHECKACCESS();

#if !USE_PPE_FINAL_RELEASE
    _heap.DumpMemoryStats();
#endif

    // destroy non-trivial type instances :
    for (FAtom& atom : _destructibles)
        atom.Traits()->Destroy(atom.Data());

    _destructibles.clear_ReleaseMemory();
    _heap.ReleaseAll();
}
//----------------------------------------------------------------------------
FAtom FAtomHeap::MakeAtomUinitialized_(const PTypeTraits& traits) {
    THIS_THREADRESOURCE_CHECKACCESS();

    const FTypeInfos typeInfos = traits->TypeInfos();
    Assert(typeInfos.SizeInBytes());

    const FAtom atom{ _heap.Allocate(typeInfos.SizeInBytes()), traits };

    // track  non-trivial type instances for destruction
    if (not (typeInfos.Flags() & ETypeFlags::TriviallyDestructible))
        _destructibles.push_back(atom);

    return atom;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
