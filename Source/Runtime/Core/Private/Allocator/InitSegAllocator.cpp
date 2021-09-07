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

#if USE_PPE_PLATFORM_DEBUG
#   include "HAL/PlatformDebug.h"
#   include "IO/TextWriter.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_PLATFORM_DEBUG
static void VeryLowLevelLog_(std::initializer_list<FConstChar> args) {
    FPlatformDebug::OutputDebug("[InitSegAllocator] ");
    for (FConstChar txt : args)
        FPlatformDebug::OutputDebug(txt);
    FPlatformDebug::OutputDebug("\n");
}
#endif //!USE_PPE_PLATFORM_DEBUG
//----------------------------------------------------------------------------
} //!namespace
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
            char id[10];
            FFixedSizeTextWriter oss{ id };
            oss << --_debugInitOrder << Eos;
            VeryLowLevelLog_({ "#", id, ": call destructor for ", _head->DebugName });
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
        char id[10];
        FFixedSizeTextWriter oss{ id };
        oss << allocator._debugInitOrder++ << Eos;
        VeryLowLevelLog_({ "#", id, ": call constructor for ", alloc.DebugName });
    }
#endif

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
