#include "stdafx.h"

#include "LinearHeap.h"

#include "Memory/MemoryDomain.h"
#include "Memory/VirtualMemory.h"
#include "Thread/AtomicSpinLock.h"
#include "Thread/ThreadContext.h"

#include "Diagnostic/Logger.h"
#ifdef USE_DEBUG_LOGGER
#   include "IO/FormatHelpers.h"
#endif

//----------------------------------------------------------------------------
// Turn to 1 to disable linear heap allocations (useful for memory debugging) :
//----------------------------------------------------------------------------
#define WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC (USE_CORE_MEMORY_DEBUGGING)

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
#   include "Allocator/Malloc.h"
#   include "Container/IntrusiveList.h"
#endif

#define WITH_CORE_LINEARHEAP_RECYCLE_CRUSTS (!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC && 1/* %__NOCOMMIT% */)
#define WITH_CORE_LINEARHEAP_RECYCLE_DELETEDS (!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC && 1/* %__NOCOMMIT% */)

namespace Core {
EXTERN_LOG_CATEGORY(CORE_API, MemoryTracking)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, GLinearHeapGranularity_, ALLOCATION_GRANULARITY); // 64k
STATIC_CONST_INTEGRAL(size_t, GLinearHeapAllocationSize, GLinearHeapGranularity_);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapBlockCapacity, GLinearHeapAllocationSize - ALLOCATION_BOUNDARY);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapMaxSize, GLinearHeapBlockCapacity);
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
#   define AssertCheckCanary(_ITEM) Assert_NoAssume((_ITEM).CheckCanary())
static void FillUninitializedBlock_(void* ptr, size_t sizeInBytes) { ::memset(ptr, 0xCC, sizeInBytes); }
static void FillDeletedBlock_(void* ptr, size_t sizeInBytes) { ::memset(ptr, 0xDD, sizeInBytes); }
#else
#   define AssertCheckCanary(_ITEM) NOOP(_ITEM)
#endif
//----------------------------------------------------------------------------
#if WITH_CORE_LINEARHEAP_RECYCLE_DELETEDS
struct FLinearHeapDeleted_ {
    size_t Size;

    FLinearHeapDeleted_* NextBucket = nullptr; // next bucket will have a larger Size
    FLinearHeapDeleted_* NextBlock = nullptr;  // all in the same bucket <=> same Size

    static void Delete(void** deleteds, void* ptr, size_t size);
    static void* Recycle(void** deleteds, size_t size);
};
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, GLinearHeapMinBlockSizeForRecycling, ROUND_TO_NEXT_16(sizeof(FLinearHeapDeleted_)));
//----------------------------------------------------------------------------
NO_INLINE void FLinearHeapDeleted_::Delete(void** deleteds, void* ptr, size_t size) {
    Assert(size >= GLinearHeapMinBlockSizeForRecycling);
    Assert(Meta::IsAligned(16, ptr));

    auto** phead = reinterpret_cast<FLinearHeapDeleted_**>(deleteds);
    auto* deleted = INPLACE_NEW(ptr, FLinearHeapDeleted_) { size };

    FLinearHeapDeleted_* prev = nullptr;
    for (FLinearHeapDeleted_* p = *phead; p; prev = p, p = p->NextBucket) {
        // insertion sort + bucketing blocks by size
        if (size == p->Size) {
            deleted->NextBlock = p->NextBlock;
            p->NextBlock = deleted;
            return;
        }
        else if (size < p->Size) {
            deleted->NextBucket = p;
            break;
        }
    }

    if (prev)
        prev->NextBucket = deleted;
    else
        *phead = deleted;
}
//----------------------------------------------------------------------------
NO_INLINE void* FLinearHeapDeleted_::Recycle(void** deleteds, size_t size) {
    auto** phead = reinterpret_cast<FLinearHeapDeleted_**>(deleteds);

    FLinearHeapDeleted_* prev = nullptr;
    for (FLinearHeapDeleted_* p = *phead; p; prev = p, p = p->NextBucket) {
        // blocks are sorted by asc size => first fit is the best fit
        if (p->Size >= size) {
            if (FLinearHeapDeleted_* s = p->NextBlock) {
                s->NextBucket = p->NextBucket;
                if (prev)
                    prev->NextBucket = s;
                else
                    *phead = s;
            }
            else if (prev) {
                prev->NextBucket = p->NextBucket;
            }
            else {
                *phead = p->NextBucket;
            }

            // don't lost the unused part of recycled block if large enough
            u8* const premain = Meta::RoundToNext((u8*)p + size, 16);
            u8* const pend = ((u8*)p + p->Size);
            if (pend - premain >= GLinearHeapMinBlockSizeForRecycling)
                Delete(deleteds, premain, pend - premain);

            ONLY_IF_ASSERT(FillUninitializedBlock_(p, size));
            return (p);
        }
    }

    return nullptr;
}
#endif //!WITH_CORE_LINEARHEAP_RECYCLE_DELETEDS
//----------------------------------------------------------------------------
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
struct FLinearHeapBlock_ {
    FLinearHeapBlock_* Next;
    u32 Offset;

    explicit FLinearHeapBlock_(FLinearHeapBlock_* next)
        : Next(next)
        , Offset(0)
    {}

#   ifdef WITH_CORE_ASSERT
    STATIC_CONST_INTEGRAL(CODE3264(u64, u32), DefaultCanary, CODE3264(0xAABBDDEEAABBDDEEull, 0xAABBDDEEul));
    CODE3264(u64, u32) Canary = DefaultCanary; // serves as padding to align on 16
    bool CheckCanary() const { return (DefaultCanary == Canary); }

    static bool Contains(const FLinearHeapBlock_* head, const FLinearHeapBlock_* blk) {
        for (; head; head = head->Next) {
            AssertCheckCanary(*head);
            if (head == blk)
                return true;
        }
        return false;
    }

#   else
    CODE3264(u64, u32) _Padding;
#   endif

    FORCE_INLINE size_t OffsetFromPtr(void* ptr) {
        const size_t off = ((u8*)ptr - (u8*)(this + 1));
        Assert(off < GLinearHeapBlockCapacity);
        return off;
    }

    FORCE_INLINE bool AliasesToBlock(void* ptr) const {
        const size_t off = ((u8*)ptr - (u8*)(this + 1));
        return (off < GLinearHeapBlockCapacity);
    }

    static FORCE_INLINE FLinearHeapBlock_* BlockFromPtr(void* ptr) {
        Assert(not Meta::IsAligned(GLinearHeapAllocationSize, ptr));
        auto* blk = static_cast<FLinearHeapBlock_*>(Meta::RoundToPrev(ptr, GLinearHeapAllocationSize));
        AssertCheckCanary(*blk);
        return blk;
    }
};
STATIC_ASSERT(sizeof(FLinearHeapBlock_) == ALLOCATION_BOUNDARY);
STATIC_ASSERT(sizeof(FLinearHeapBlock_) + GLinearHeapBlockCapacity == GLinearHeapGranularity_);
#else
struct FLinearHeapHeader_ {
    FLinearHeap* const Owner;

    TIntrusiveListNode<FLinearHeapHeader_> Node;
    typedef INTRUSIVELIST_ACCESSOR(&FLinearHeapHeader_::Node) list_type;

#ifdef WITH_CORE_ASSERT
    const u32 SizeInBytes;
    const u32 Canary;
    STATIC_CONST_INTEGRAL(size_t, GDefaultCanary, 0xAABBCCDD);
    bool CheckCanary() const { return (GDefaultCanary == Canary); }
#else
    const size_t SizeInBytes;
#endif

    FLinearHeapHeader_(FLinearHeap& owner, size_t sizeInBytes)
        : Owner(&owner)
#ifdef WITH_CORE_ASSERT
        , SizeInBytes(checked_cast<u32>(sizeInBytes))
        , Canary(GDefaultCanary)
#else
        , SizeInBytes(sizeInBytes)
#endif
    {}

    void Add(void** pHead) {
        list_type::PushFront(reinterpret_cast<FLinearHeapHeader_**>(pHead), nullptr, this);
    }

    void Erase(void** pHead) {
        list_type::Erase(reinterpret_cast<FLinearHeapHeader_**>(pHead), nullptr, this);
    }

    void* data() { return (this + 1); }
    static FLinearHeapHeader_& FromData(void* ptr) {
        auto& header = (reinterpret_cast<FLinearHeapHeader_*>(ptr)[-1]);
        AssertCheckCanary(header);
        return header;
    }
};
STATIC_ASSERT(Meta::IsAligned(16, sizeof(FLinearHeapHeader_)));
#endif //!!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
//----------------------------------------------------------------------------
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
class FLinearHeapVMCache_ {
public:
    static NO_INLINE FLinearHeapBlock_* AllocateBlock(FLinearHeapBlock_* next) {
        void* const ptr = Instance_().Allocate_(GLinearHeapAllocationSize);
        FLinearHeapBlock_* blk = INPLACE_NEW(ptr, FLinearHeapBlock_){ next };
#ifdef WITH_CORE_ASSERT
        ::memset(blk + 1, 0xCC, GLinearHeapBlockCapacity);
#endif
        return blk;
    }

    static FLinearHeapBlock_* ReleaseBlock(FLinearHeapBlock_* blk) {
        AssertCheckCanary(*blk);
        Assert(blk->Offset <= GLinearHeapBlockCapacity);
        FLinearHeapBlock_* const next = blk->Next;
        Instance_().Free_(blk, GLinearHeapAllocationSize);
        return next;
    }

    static void ReleaseAll() {
        Instance_().ReleaseAll_();
    }

private:
    STATIC_CONST_INTEGRAL(size_t, VMCacheBlocks, 32);
    STATIC_CONST_INTEGRAL(size_t, VMCacheSizeInBytes, 2 * 1024 * 1024); // <=> 2 mo global cache for large blocks

    FAtomicSpinLock _barrier;
    VIRTUALMEMORYCACHE(LinearHeap, VMCacheBlocks, VMCacheSizeInBytes) _vm;

    static FLinearHeapVMCache_& Instance_() {
        static FLinearHeapVMCache_ GInstance;
        return GInstance;
    }

    void* Allocate_(size_t sizeInBytes) {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        return _vm.Allocate(sizeInBytes);
    }

    void Free_(void* ptr, size_t sizeInBytes) {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        _vm.Free(ptr, sizeInBytes);
    }

    void ReleaseAll_() {
        const FAtomicSpinLock::FScope scopeLock(_barrier);
        _vm.ReleaseAll();
    }
};
#endif //!!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const size_t FLinearHeap::MaxBlockSize = GLinearHeapMaxSize;
//----------------------------------------------------------------------------
#ifdef USE_MEMORY_DOMAINS
FLinearHeap::FLinearHeap(FMemoryTracking* parent)
#else
FLinearHeap::FLinearHeap()
#endif
    : _blocks(nullptr)
    , _deleteds(nullptr)
#ifdef USE_MEMORY_DOMAINS
    , _trackingData("LinearHeap", parent) {
    RegisterAdditionalTrackingData(&_trackingData);
}
#else
{}
#endif
//----------------------------------------------------------------------------
FLinearHeap::~FLinearHeap() {
    ReleaseAll();
#ifdef USE_MEMORY_DOMAINS
    UnregisterAdditionalTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
void* FLinearHeap::Allocate(size_t size) {
    Assert(size);
    AssertRelease(size <= MaxBlockSize);

    size = SnapSize(size);

    void* ptr;

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    void* stg = Core::aligned_malloc(sizeof(FLinearHeapHeader_) + size, alignment);
    auto* blk = INPLACE_NEW(stg, FLinearHeapHeader_)(*this, size);
    AssertCheckCanary(*blk);

    blk->Add(&_blocks);

    ptr = blk->data();

#else
    auto* blk = static_cast<FLinearHeapBlock_*>(_blocks);
    if (Unlikely(nullptr == blk || blk->Offset + size > GLinearHeapBlockCapacity)) {
        FLinearHeapBlock_* prev = blk;
        blk = nullptr;

#if WITH_CORE_LINEARHEAP_RECYCLE_DELETEDS
        // don't lost remaining memory in current chunk
        if (prev && (prev->Offset + GLinearHeapMinBlockSizeForRecycling) <= GLinearHeapBlockCapacity)
            FLinearHeapDeleted_::Delete(&_deleteds, &reinterpret_cast<u8*>(prev + 1)[prev->Offset], GLinearHeapBlockCapacity - prev->Offset);

        // try to look for a fit in released blocks linked list
        if (void* ptr = (_deleteds ? FLinearHeapDeleted_::Recycle(&_deleteds, size) : nullptr))
            return ptr;

#elif WITH_CORE_LINEARHEAP_RECYCLE_CRUSTS
        // try to look in previous blocks in there's enough space there
        if (prev) {
            for (blk = prev->Next; blk; blk = blk->Next) {
                if (Meta::RoundToNext(blk->Offset, alignment) + size <= GLinearHeapBlockCapacity)
                    break;
            }
        }

#endif

        // finally can't avoid allocating a new block for the heap
        if (nullptr == blk) // could have been recycled ^^^
            _blocks = blk = FLinearHeapVMCache_::AllocateBlock(prev);
    }


    AssertCheckCanary(*blk);

    const size_t off = blk->Offset;
    Assert(off + size <= GLinearHeapBlockCapacity);

    blk->Offset = checked_cast<u32>(off + size);

    ptr = (&reinterpret_cast<u8*>(blk + 1)[off]);

#endif

    Assert(ptr);
    Assert(Meta::IsAligned(16, ptr));
#ifdef USE_MEMORY_DOMAINS
    _trackingData.Allocate(1, size);
#endif

    return ptr;
}
//----------------------------------------------------------------------------
void* FLinearHeap::Relocate(void* ptr, size_t newSize, size_t oldSize) {
    Assert(newSize);
    AssertRelease(newSize <= MaxBlockSize);

    if (nullptr == ptr) {
        Assert(0 == oldSize);
        return Allocate(newSize);
    }

    oldSize = SnapSize(oldSize);
    newSize = SnapSize(newSize);

    if (oldSize == newSize)
        return ptr;

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    FLinearHeapHeader_& prev = FLinearHeapHeader_::FromData(ptr);
    AssertRelease(prev.Owner == this);
    AssertRelease(oldSize == prev.SizeInBytes);

    prev.Erase(&_blocks);

    void* const newp = Allocate(newSize, alignment);
    ::memcpy(newp, ptr, Min(oldSize, newSize));

#   ifdef USE_MEMORY_DOMAINS
    _trackingData.Deallocate(1, oldSize);
#   endif

    Core::aligned_free(&prev);

    return newp;

#else

    FLinearHeapBlock_* blk = FLinearHeapBlock_::BlockFromPtr(ptr);
    Assert_NoAssume(FLinearHeapBlock_::Contains(static_cast<FLinearHeapBlock_*>(_blocks), blk));

    const size_t off = blk->OffsetFromPtr(ptr);
    if (blk->Offset == off + oldSize && off + newSize <= GLinearHeapBlockCapacity) {
        blk->Offset = checked_cast<u32>(off + newSize);
#ifdef USE_MEMORY_DOMAINS
        _trackingData.Deallocate(1, oldSize);
        _trackingData.Allocate(1, newSize);
#endif
        return ptr;
    }
    else {
        void* const newp = Allocate(newSize);
        ::memcpy(newp, ptr, Min(oldSize, newSize));
        Release(ptr, oldSize);
        return newp;
    }

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::Release(void* ptr, size_t size) {
    Assert(nullptr != ptr);
    Assert(0 != size);
    Assert(Meta::IsAligned(16, ptr));
    AssertRelease(size <= MaxBlockSize);

    size = SnapSize(size);

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    FLinearHeapHeader_& blk = FLinearHeapHeader_::FromData(ptr);
    AssertRelease(blk.Owner == this);
    AssertRelease(SnapSize(size) == blk.SizeInBytes);

    blk.Erase(&_blocks);

#   ifdef USE_MEMORY_DOMAINS
    _trackingData.Deallocate(1, size);
#   endif

    Core::aligned_free(&blk);

#else
    ONLY_IF_ASSERT(FillDeletedBlock_(ptr, size));

    FLinearHeapBlock_* const blk = FLinearHeapBlock_::BlockFromPtr(ptr);
    Assert_NoAssume(FLinearHeapBlock_::Contains(static_cast<FLinearHeapBlock_*>(_blocks), blk));

    const size_t off = blk->OffsetFromPtr(ptr);
    if (off + size == blk->Offset) {
        // reclaim memory
        blk->Offset = checked_cast<u32>(off);
    }
#   if WITH_CORE_LINEARHEAP_RECYCLE_DELETEDS
    else if (size >= GLinearHeapMinBlockSizeForRecycling) {
        FLinearHeapDeleted_::Delete(&_deleteds, ptr, size);
    }
#   endif
    /*
    else {
        // just forget about this block, it's too small to be worth recycling
    }
    */

#   ifdef USE_MEMORY_DOMAINS
    _trackingData.Deallocate(1, size);
#   endif

#endif
}

//----------------------------------------------------------------------------
void* FLinearHeap::Relocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize) {
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    Assert(ptr);

    auto& header = FLinearHeapHeader_::FromData(ptr);
    AssertRelease(&header == _blocks); // last block allocated

    return Relocate(ptr, newSize, header.SizeInBytes, alignment);

#else
    Assert(ptr); // you should use Allocate() instead
    Assert(newSize);
    Assert(oldSize);
    AssertRelease(newSize <= MaxBlockSize);

    newSize = SnapSize(newSize);
    oldSize = SnapSize(oldSize);

    if (oldSize == newSize)
        return ptr;

    FLinearHeapBlock_* blk = FLinearHeapBlock_::BlockFromPtr(ptr);
    Assert_NoAssume(FLinearHeapBlock_::Contains(static_cast<FLinearHeapBlock_*>(_blocks), blk));

    const size_t off = blk->OffsetFromPtr(ptr);
    Assert(off + oldSize == blk->Offset);

    if (off + newSize <= GLinearHeapBlockCapacity) {
        blk->Offset = checked_cast<u32>(off + newSize);
#ifdef USE_MEMORY_DOMAINS
        _trackingData.Deallocate(1, oldSize);
        _trackingData.Allocate(1, newSize);
#endif
        return ptr;
    }
    else {
        void* const newp = Allocate(newSize);
        ::memcpy(newp, ptr, Min(oldSize, newSize));
        Release(ptr, oldSize);
        return newp;
    }

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::Release_AssumeLast(void* ptr, size_t size) {
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    Assert(ptr);

    auto& header = FLinearHeapHeader_::FromData(ptr);
    AssertRelease(&header == _blocks); // last block allocated

    return Release(ptr, size);

#else
    Assert(nullptr != ptr);
    Assert(size);

    size = SnapSize(size);

    FLinearHeapBlock_* blk = FLinearHeapBlock_::BlockFromPtr(ptr);
    Assert_NoAssume(FLinearHeapBlock_::Contains(static_cast<FLinearHeapBlock_*>(_blocks), blk));

    const size_t off = blk->OffsetFromPtr(ptr);
    Assert(off + size == blk->Offset);

    // reclaim memory
    blk->Offset = checked_cast<u32>(off);

#endif
}
//----------------------------------------------------------------------------
bool FLinearHeap::AliasesToHeap(void* ptr) const {
    Assert(ptr);

    // !!! SLOW !!! (but safe)

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    for (auto* blk = static_cast<FLinearHeapHeader_*>(_blocks); blk; blk = blk->Node.Next) {
        AssertCheckCanary(*blk);
        if (blk->data() == ptr)
            return true;
    }
    return false;

#else
    for (auto* blk = static_cast<const FLinearHeapBlock_*>(_blocks); blk; blk = blk->Next) {
        AssertCheckCanary(*blk);
        if (blk->AliasesToBlock(ptr))
            return true;
    }
    return false;

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::ReleaseAll() {
    if (nullptr == _blocks) {
        Assert(nullptr == _deleteds);
        return; // nothing to do
    }

#if defined(USE_DEBUG_LOGGER) && defined(USE_MEMORY_DOMAINS)
    size_t totalBlockCount = 0;
    size_t totalSizeInBytes = 0;

#   if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    for (auto* blk = static_cast<FLinearHeapHeader_*>(_blocks); blk; blk = blk->Node.Next) {
        totalBlockCount++;
        totalSizeInBytes += blk->SizeInBytes;
    }
#   else
    for (auto* blk = static_cast<FLinearHeapBlock_*>(_blocks); blk; blk = blk->Next) {
        totalBlockCount++;
        totalSizeInBytes += GLinearHeapBlockCapacity;
    }
#   endif

    LOG(MemoryTracking, Debug, L"linear heap <{0}> allocated {1} blocks using {2} pages, ie {3:f2} / {4:f2} allocated ({5}, max:{6})",
        _trackingData.Parent()
            ? MakeCStringView(_trackingData.Parent()->Name())
            : MakeCStringView(_trackingData.Name()),
        Fmt::CountOfElements(_trackingData.BlockCount()),
        totalBlockCount,
        Fmt::SizeInBytes(_trackingData.TotalSizeInBytes()),
        Fmt::SizeInBytes(totalSizeInBytes),
        Fmt::Percentage(_trackingData.TotalSizeInBytes(), totalSizeInBytes),
        Fmt::Percentage(_trackingData.MaxTotalSizeInBytes(), totalSizeInBytes));

#   if WITH_CORE_LINEARHEAP_RECYCLE_DELETEDS
    totalBlockCount = 0;
    totalSizeInBytes = 0;
    size_t maxBlockSize = 0;
    size_t minBlockSize = CODE3264(UINT32_MAX, UINT64_MAX);
    for (auto* blk = static_cast<FLinearHeapDeleted_*>(_deleteds); blk; blk = blk->NextBucket) {
        size_t blockCount = 0;
        size_t sizeInBytes = 0;
        for (auto* p = blk; p; p = p->NextBlock) {
            blockCount++;
            sizeInBytes += p->Size;
        }

        LOG(MemoryTracking, Debug, L"linear heap <{0}> deleted [{1:4}] : {2} blocks ({3} total memory)",
            _trackingData.Parent()
                ? MakeCStringView(_trackingData.Parent()->Name())
                : MakeCStringView(_trackingData.Name()),
            blk->Size,
            Fmt::CountOfElements(blockCount),
            Fmt::SizeInBytes(sizeInBytes) );

        maxBlockSize = Max(maxBlockSize, blk->Size);
        minBlockSize = Min(minBlockSize, blk->Size);
        totalBlockCount += blockCount;
        totalSizeInBytes += sizeInBytes;
    }

    LOG(MemoryTracking, Debug, L"linear heap <{0}> had {1} deleted blocks, total of {2} recyclable ({3} -> {4})",
        _trackingData.Parent()
            ? MakeCStringView(_trackingData.Parent()->Name())
            : MakeCStringView(_trackingData.Name()),
        totalBlockCount,
        Fmt::SizeInBytes(totalSizeInBytes),
        Fmt::SizeInBytes(minBlockSize),
        Fmt::SizeInBytes(maxBlockSize) );
#   endif
#endif

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    Assert(nullptr == _deleteds);

    while (_blocks)
        Release_AssumeLast(static_cast<FLinearHeapHeader_*>(_blocks)->data());

    Assert_NoAssume(_trackingData.AllocationCount() == 0);
    Assert(nullptr == _blocks);

#else
    for (auto* blk = static_cast<FLinearHeapBlock_*>(_blocks); blk; )
        blk = FLinearHeapVMCache_::ReleaseBlock(blk);

    _blocks = nullptr;
    _deleteds = nullptr;
#   ifdef USE_MEMORY_DOMAINS
    _trackingData.ReleaseAll();
#   endif

#endif
}
//----------------------------------------------------------------------------
void FLinearHeap::FlushVirtualMemoryCache() {
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    FLinearHeapVMCache_::ReleaseAll();
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#undef AssertCheckCanary
