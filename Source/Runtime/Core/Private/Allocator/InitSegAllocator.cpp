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
namespace {
//----------------------------------------------------------------------------
// should be in compiler segment, so destroyed last hopefully
static FInitSegAllocator GInitSegAllocator_ INITSEG_COMPILER_PRIORITY;
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FInitSegAllocator::FInitSegAllocator() = default;
//----------------------------------------------------------------------------
FInitSegAllocator::~FInitSegAllocator() {
    const FAtomicOrderedLock::FScope scopeLock(_barrier);
    for (; _head; _head = _head->Next)
        _head->Deleter(_head->Data);
}
//----------------------------------------------------------------------------
FInitSegAllocator& FInitSegAllocator::Get() NOEXCEPT {
    return GInitSegAllocator_;
}
//----------------------------------------------------------------------------
void FInitSegAllocator::Allocate(FAlloc& alloc) NOEXCEPT {
    Assert(nullptr == alloc.Next);
    const FAtomicOrderedLock::FScope scopeLock(_barrier);
    if (Likely(_head)) {
        _tail->Next = &alloc;
        _tail = &alloc;
    }
    else {
        _head = _tail = &alloc;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
