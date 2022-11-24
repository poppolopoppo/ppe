// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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

#if USE_PPE_PLATFORM_DEBUG
#   include "HAL/PlatformDebug.h"
#   include "IO/Format.h"
#   include "IO/TextWriter.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FInitSegAllocator::FInitSegAllocator() = default;
// should be in compiler segment, so destroyed last hopefully
FInitSegAllocator FInitSegAllocator::GInitSegAllocator_ INITSEG_COMPILER_PRIORITY;
//----------------------------------------------------------------------------
FInitSegAllocator::~FInitSegAllocator() {
    const FAtomicOrderedLock::FScope scopeLock(_barrier);

    for (; _head; _head = _head->Next) {
#if USE_PPE_PLATFORM_DEBUG
        {
            Assert(_debugInitOrder > 0);
            --_debugInitOrder;

            char mesg[200];
            Format(mesg, "[InitSegAllocator][#{0:#2}] -- {1:#8} -- call destructor for {2}\n",
                _debugInitOrder, _head->Priority, _head->DebugName );
            FPlatformDebug::OutputDebug(mesg);
        }
#endif

        _head->Deleter(*_head);
    }
}
//----------------------------------------------------------------------------
void FInitSegAllocator::Allocate(FAlloc& alloc) NOEXCEPT {
    Assert(nullptr == alloc.Next);

    FInitSegAllocator& allocator = GInitSegAllocator_;
    const FAtomicOrderedLock::FScope scopeLock(allocator._barrier);

#if USE_PPE_PLATFORM_DEBUG
    {
        char mesg[200];
        Format(mesg, "[InitSegAllocator][#{0:#2}] -- {1:#8} -- call constructor for {2}\n",
            allocator._debugInitOrder, alloc.Priority, alloc.DebugName );
        ++allocator._debugInitOrder;
        FPlatformDebug::OutputDebug(mesg);
    }
#endif

    FAlloc* lower_bound = nullptr;
    for (FAlloc* p = allocator._head; p && p->Priority >= alloc.Priority; p = p->Next)
        lower_bound = p;

    if (lower_bound) {
        Assert(allocator._head);
        alloc.Next = lower_bound->Next;
        lower_bound->Next = &alloc;
    }
    else {
        alloc.Next = allocator._head;
        allocator._head = &alloc;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
