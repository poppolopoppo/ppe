#include "stdafx.h"

#include "Token.h"

#include "Allocator/VirtualMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Misc/TargetPlatform.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
using FChunk = FTokenFactory::FChunk;
using FEntry = FTokenFactory::FEntry;
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, PageSize, HUGE_PAGE_SIZE);
//----------------------------------------------------------------------------
static FEntry* AllocateEntry_(FChunk** chunks, size_t len, size_t stride, size_t hash) {
    Assert(len > 0);
    Assert(stride > 0);

    constexpr size_t chunkSizeInBytes = (PageSize - sizeof(FChunk));
    const size_t allocSizeInBytes = (sizeof(FEntry) + (len + 1/* null terminate the string */) * stride);

    FChunk* chunkToAllocate = nullptr;
    for (FChunk* chunk = *chunks; chunk; chunk = chunk->Next) {
        Assert(chunk->TestCanary());

        if (chunk->WriteOffset + allocSizeInBytes <= chunkSizeInBytes) {
            chunkToAllocate = chunk;
            break;
        }
    }

    if (nullptr == chunkToAllocate) {
        chunkToAllocate = new (FVirtualMemory::AlignedAlloc(PageSize, PageSize)) FChunk();
        chunkToAllocate->Next = *chunks;
        *chunks = chunkToAllocate;
    }

    Assert(chunkToAllocate);
    Assert(chunkToAllocate->TestCanary());
    Assert(chunkToAllocate->WriteOffset + allocSizeInBytes <= chunkSizeInBytes);

    FEntry* entry = new (chunkToAllocate->Data() + chunkToAllocate->WriteOffset) FEntry(len, hash);
    chunkToAllocate->WriteOffset = checked_cast<u32>(chunkToAllocate->WriteOffset + allocSizeInBytes);

#ifdef USE_MEMORY_DOMAINS
    MEMORY_DOMAIN_TRACKING_DATA(Token).Allocate(1, allocSizeInBytes);
#endif

    return entry;
}
//----------------------------------------------------------------------------
static void ReleaseChunks_(FChunk* chunks) {
    while (chunks) {
        Assert(chunks->TestCanary());
        FChunk* const next = chunks->Next;
        FVirtualMemory::AlignedFree(chunks, PageSize);
        chunks = next;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTokenFactory::FTokenFactory() 
    : _chunks(nullptr) {}
//----------------------------------------------------------------------------
FTokenFactory::~FTokenFactory() {
    const std::unique_lock<std::mutex> scopeLock(_barrier);
    ReleaseChunks_(_chunks);
}
//----------------------------------------------------------------------------
const FTokenFactory::FEntry* FTokenFactory::Lookup(size_t len, size_t hash, const FEntry* head) const {
    head = ((nullptr == head))
        ? _bucketHeads[hash & MaskBuckets]
        : head->Next;

    for (const FEntry* bucket = head; bucket; bucket = bucket->Next) {
        Assert(bucket->TestCanary());

        if (bucket->HashValue == hash && bucket->Length == len)
            return bucket;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
const FTokenFactory::FEntry* FTokenFactory::Allocate(void* src, size_t len, size_t stride, size_t hash, const FEntry* tail) {
    const std::unique_lock<std::mutex> scopeLock(_barrier);

    FEntry* result = const_cast<FEntry*>(Lookup(len, hash, tail)); // check if wasn't allocated by another thread
    if (result)
        return result;

    const size_t bucket = (hash & MaskBuckets);
    FEntry* head = _bucketHeads[bucket];
    tail = _bucketTails[bucket];

    result = AllocateEntry_(&_chunks, len, stride, hash);
    ::memcpy(result->Data(), src, len * stride);
    ::memset(result->Data() + len * stride, 0x00, stride); // null terminate

    if (nullptr == head) {
        Assert(nullptr == tail);
        
        if (FPlatformAtomics::CompareExchange((void**)&_bucketHeads[bucket], result, head) != head)
            Assert("concurrency problem, some thread might already have the head", 0);
    }
    else {
        Assert(nullptr != tail);
        Assert(head->TestCanary());
        Assert(tail->TestCanary());

        if (FPlatformAtomics::CompareExchange((void**)&tail->Next, result, nullptr) != nullptr)
            Assert("concurrency problem, some thread might already have the tail", 0);
    }

    _bucketTails[bucket] = result; // non-atomically updated since only used when locked

    Assert(result->TestCanary());
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core