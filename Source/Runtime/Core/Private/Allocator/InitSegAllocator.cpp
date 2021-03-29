#include "stdafx.h"

#include "Allocator/InitSegAllocator.h"

// this allocator should solve static var initialization fiasco,
// and tie the lifetime of every static variables together.

PRAGMA_INITSEG_COMPILER

// profit from this isolated TU to define our new/delete overloads
#include "Allocator/New.h"
#if PPE_OVERRIDE_NEW_ONCE
//  when compiling statically without inline new operators it must be defined once in a separate TU
#   include "Allocator/New.Definitions-inl.h"
#endif

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, InitSeg)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FInitSegAllocator::FInitSegAllocator() = default;
// should be in compiler segment, so destroyed last hopefully
FInitSegAllocator FInitSegAllocator::GInitSegAllocator_ INITSEG_COMPILER_PRIORITY;
//----------------------------------------------------------------------------
FInitSegAllocator::~FInitSegAllocator() {
    const FAtomicOrderedLock::FScope scopeLock(_barrier);

    for (; _head; _head = _head->Next)
        _head->Deleter(*_head);
}
//----------------------------------------------------------------------------
void FInitSegAllocator::Allocate(FAlloc& alloc) NOEXCEPT {
    Assert(nullptr == alloc.Next);

    FInitSegAllocator& allocator = GInitSegAllocator_;
    const FAtomicOrderedLock::FScope scopeLock(allocator._barrier);

    if (Likely(allocator._head)) {
        allocator._tail->Next = &alloc;
        allocator._tail = &alloc;
    }
    else {
        allocator._head = allocator._tail = &alloc;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
