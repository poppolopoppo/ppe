#include "stdafx.h"

#include "Token.h"

#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/VirtualMemory.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTokenFactory::FTokenFactory()
{}
//----------------------------------------------------------------------------
FTokenFactory::~FTokenFactory() {
    const Meta::FLockGuard scopeLock(_barrier);
    _heap.ReleaseAll();
}
//----------------------------------------------------------------------------
const FTokenFactory::FEntry* FTokenFactory::Lookup(size_t len, size_t hash, const FEntry* head) const {
    head = ((nullptr == head))
        ? _bucketHeads[hash & MaskBuckets]
        : head->Next;

    for (const FEntry* bucket = head; bucket; bucket = bucket->Next) {
        Assert_NoAssume(bucket->TestCanary());

        if (bucket->HashValue == hash && bucket->Length == len)
            return bucket;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
const FTokenFactory::FEntry* FTokenFactory::Allocate(void* src, size_t len, size_t stride, size_t hash, const FEntry* tail) {
    FEntry* result = const_cast<FEntry*>(Lookup(len, hash, tail)); // check if wasn't allocated by another thread
    if (result)
        return result;

    const Meta::FLockGuard scopeLock(_barrier);

    const size_t bucket = (hash & MaskBuckets);
    FEntry* head = _bucketHeads[bucket];
    tail = _bucketTails[bucket];

    const size_t sizeInBytes = (sizeof(FEntry) + (len + 1/* null terminate the string */) * stride);

    result = INPLACE_NEW(_heap.Allocate(sizeInBytes), FEntry)(len, hash);
    Assert(Meta::IsAligned(std::alignment_of_v<FEntry>, result));

    FPlatformMemory::Memcpy(result->Data(), src, len * stride);
    FPlatformMemory::Memzero(result->Data() + len * stride, stride); // null terminate

    if (nullptr == head) {
        Assert(nullptr == tail);

        // if failed : concurrency problem, some thread might already have the head
        Verify(FPlatformAtomics::CompareExchangePtr((volatile void**)&_bucketHeads[bucket], (void*)result, (void*)head) == head);
    }
    else {
        Assert(nullptr != tail);
        Assert_NoAssume(head->TestCanary());
        Assert_NoAssume(tail->TestCanary());

        // if failed : concurrency problem, some thread might already have the tail
        Verify(FPlatformAtomics::CompareExchangePtr((volatile void**)&tail->Next, (void*)result, (void*)nullptr) == nullptr);
    }

    _bucketTails[bucket] = result; // non-atomically updated since only used when locked

    Assert_NoAssume(result->TestCanary());
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
