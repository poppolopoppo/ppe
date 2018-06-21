#include "stdafx.h"

#include "LinearHeap.h"

#include "Memory/MemoryTracking.h"
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

namespace Core {
EXTERN_LOG_CATEGORY(CORE_API, MemoryDomain)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_CONST_INTEGRAL(size_t, GLinearHeapGranularity_, ALLOCATION_GRANULARITY); // 64k
STATIC_CONST_INTEGRAL(size_t, GLinearHeapAllocationSize, GLinearHeapGranularity_);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapBlockCapacity, GLinearHeapAllocationSize - ALLOCATION_BOUNDARY);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapMaxSize, GLinearHeapBlockCapacity);
STATIC_CONST_INTEGRAL(size_t, GLinearHeapMinBlockSizeForRecycling, CODE3264(16, 32));
//----------------------------------------------------------------------------
#ifdef WITH_CORE_ASSERT
#   define AssertCheckCanary(_ITEM) Assert_NoAssume((_ITEM).CheckCanary())
static void FillUninitializedBlock_(void* ptr, size_t sizeInBytes) { ::memset(ptr, 0xCC, sizeInBytes); }
static void FillDeletedBlock_(void* ptr, size_t sizeInBytes) { ::memset(ptr, 0xDD, sizeInBytes); }
#else
#   define AssertCheckCanary(_ITEM) NOOP(_ITEM)
#endif
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
        void* const ptr = Get_().Allocate_(GLinearHeapAllocationSize);
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
        Get_().Free_(blk, GLinearHeapAllocationSize);
        return next;
    }

    static void ReleaseAll() {
        Get_().ReleaseAll_();
    }

private:
    STATIC_CONST_INTEGRAL(size_t, VMCacheBlocks, 32);
    STATIC_CONST_INTEGRAL(size_t, VMCacheSizeInBytes, 2 * 1024 * 1024); // <=> 2 mo global cache for large blocks

    FAtomicSpinLock _barrier;
    VIRTUALMEMORYCACHE(LinearHeap, VMCacheBlocks, VMCacheSizeInBytes) _vm;

    static FLinearHeapVMCache_& Get_() {
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
//----------------------------------------------------------------------------
struct FLinearHeapDeleted_ {
    size_t Size;

    FLinearHeapDeleted_* NextBucket = nullptr; // next bucket will have a larger Size
    FLinearHeapDeleted_* NextBlock = nullptr;  // all in the same bucket <=> same Size

#ifdef WITH_CORE_ASSERT
    static constexpr size_t DefaultCanary = CODE3264(0xDEADCAFEul, 0xDEADCAFEF001BEEFull);
    const size_t Canary = DefaultCanary;
    bool CheckCanary() const { return (DefaultCanary == Canary); }
#endif

    static void Delete(void** deleteds, void* ptr, size_t size);
    static void* Recycle(void** deleteds, size_t size);

    static NO_INLINE void* TryAllocate(void** deleteds, size_t size, FLinearHeapBlock_* prev) {
        // don't lost remaining memory in current chunk
        if (prev && (prev->Offset + GLinearHeapMinBlockSizeForRecycling) <= GLinearHeapBlockCapacity) {
            FLinearHeapDeleted_::Delete(deleteds, &reinterpret_cast<u8*>(prev + 1)[prev->Offset], GLinearHeapBlockCapacity - prev->Offset);
            prev->Offset = GLinearHeapBlockCapacity;
        }

        // try to look for a fit in released blocks linked list
        return (deleteds
            ? FLinearHeapDeleted_::Recycle(deleteds, size)
            : nullptr);
    }
};
STATIC_ASSERT(ROUND_TO_NEXT_16(sizeof(FLinearHeapDeleted_)) == GLinearHeapMinBlockSizeForRecycling);
//----------------------------------------------------------------------------
void FLinearHeapDeleted_::Delete(void** deleteds, void* ptr, size_t size) {
    Assert(size >= GLinearHeapMinBlockSizeForRecycling);
    Assert(Meta::IsAligned(16, ptr));

    auto** phead = reinterpret_cast<FLinearHeapDeleted_**>(deleteds);
    auto* deleted = INPLACE_NEW(ptr, FLinearHeapDeleted_) { size };

    Assert(nullptr == deleted->NextBucket);
    Assert(nullptr == deleted->NextBlock);
    Assert_NoAssume(deleted->CheckCanary());

    FLinearHeapDeleted_* prev = nullptr;
    for (FLinearHeapDeleted_* p = *phead; p; prev = p, p = p->NextBucket) {
        Assert(ptr != p);
        Assert_NoAssume(p->CheckCanary());

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
void* FLinearHeapDeleted_::Recycle(void** deleteds, size_t size) {
    auto** phead = reinterpret_cast<FLinearHeapDeleted_**>(deleteds);

    FLinearHeapDeleted_* prev = nullptr;
    for (FLinearHeapDeleted_* p = *phead; p; prev = p, p = p->NextBucket) {
        Assert_NoAssume(p->CheckCanary());

        // blocks are sorted by asc size => first fit is the best fit
        if (p->Size >= size) {
            if (FLinearHeapDeleted_* s = p->NextBlock) {
                Assert(s != p);
                Assert_NoAssume(s->CheckCanary());

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

            // recycle reminder if it's large enough,
            // note the memory will be lost it's too small !!!
            // TODO : test avoiding recycling when the reminder will be too small
            if (p->Size >= size + GLinearHeapMinBlockSizeForRecycling) {
                Assert(Meta::IsAligned(16, p));

                u8* const premain = ((u8*)p + size);
                u8* const pend = ((u8*)p + p->Size);
                Delete(deleteds, premain, pend - premain);
            }

            ONLY_IF_ASSERT(const size_t oldSize = p->Size);
            ONLY_IF_ASSERT(FillUninitializedBlock_(p, size));
            Assert_NoAssume(oldSize < size + GLinearHeapMinBlockSizeForRecycling || 
                ((FLinearHeapDeleted_*)((u8*)p + size))->CheckCanary() );
            return (p);
        }
    }

    return nullptr;
}
#endif //!!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const size_t FLinearHeap::MinBlockSize = ALLOCATION_BOUNDARY;
const size_t FLinearHeap::MinBlockSizeForRecycling = GLinearHeapMinBlockSizeForRecycling;
const size_t FLinearHeap::MaxBlockSize = GLinearHeapMaxSize;
//----------------------------------------------------------------------------
#if USE_CORE_MEMORYDOMAINS
FLinearHeap::FLinearHeap(FMemoryTracking& parent)
#else
FLinearHeap::FLinearHeap()
#endif
    : _numAllocs(0)
    , _blocks(nullptr)
    , _deleteds(nullptr)
#if USE_CORE_MEMORYDOMAINS
    , _trackingData("LinearHeap", &parent) {
    Assert(parent.IsChildOf(FMemoryTracking::UsedMemory()));
    RegisterTrackingData(&_trackingData);
}
#else
{}
#endif
//----------------------------------------------------------------------------
FLinearHeap::~FLinearHeap() {
    ReleaseAll();
#if USE_CORE_MEMORYDOMAINS
    UnregisterTrackingData(&_trackingData);
#endif
}
//----------------------------------------------------------------------------
void* FLinearHeap::Allocate(size_t size) {
    Assert(size);
    AssertRelease(size <= MaxBlockSize);

    size = SnapSize(size);

    void* ptr;

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    void* stg = Core::aligned_malloc(sizeof(FLinearHeapHeader_) + size, 16);
    auto* blk = INPLACE_NEW(stg, FLinearHeapHeader_)(*this, size);
    AssertCheckCanary(*blk);

    blk->Add(&_blocks);

    ptr = blk->data();

#else
    auto* blk = static_cast<FLinearHeapBlock_*>(_blocks);
    if (Unlikely(nullptr == blk || blk->Offset + size > GLinearHeapBlockCapacity)) {
        // try to look for a fit in released blocks linked list
        if (void* ptr = FLinearHeapDeleted_::TryAllocate(&_deleteds, size, blk))
            return ptr;

        // finally can't avoid allocating a new block for the heap
        _blocks = blk = FLinearHeapVMCache_::AllocateBlock(blk);
    }

    AssertCheckCanary(*blk);

    const size_t off = blk->Offset;
    Assert(off + size <= GLinearHeapBlockCapacity);

    blk->Offset = checked_cast<u32>(off + size);

    ptr = (&reinterpret_cast<u8*>(blk + 1)[off]);

#endif

    Assert(ptr);
    Assert(Meta::IsAligned(16, ptr));
#if USE_CORE_MEMORYDOMAINS
    _trackingData.Allocate(size / 16, 16);
#endif
    _numAllocs++;

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

    void* const newp = Allocate(newSize);
    ::memcpy(newp, ptr, Min(oldSize, newSize));

#   if USE_CORE_MEMORYDOMAINS
    _trackingData.Deallocate(oldSize / 16, 16);
#   endif

    Core::aligned_free(&prev);

    return newp;

#else

    FLinearHeapBlock_* blk = FLinearHeapBlock_::BlockFromPtr(ptr);
    Assert_NoAssume(FLinearHeapBlock_::Contains(static_cast<FLinearHeapBlock_*>(_blocks), blk));

    const size_t off = blk->OffsetFromPtr(ptr);
    if (blk->Offset == off + oldSize && off + newSize <= GLinearHeapBlockCapacity) {
        blk->Offset = checked_cast<u32>(off + newSize);
#if USE_CORE_MEMORYDOMAINS
        _trackingData.Deallocate(oldSize / 16, 16);
        _trackingData.Allocate(newSize / 16, 16);
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
    Assert(_numAllocs);

    _numAllocs--;
    size = SnapSize(size);

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    FLinearHeapHeader_& blk = FLinearHeapHeader_::FromData(ptr);
    AssertRelease(blk.Owner == this);
    AssertRelease(SnapSize(size) == blk.SizeInBytes);

    blk.Erase(&_blocks);

#   if USE_CORE_MEMORYDOMAINS
    _trackingData.Deallocate(size / 16, 16);
#   endif

    Core::aligned_free(&blk);

#else
    ONLY_IF_ASSERT(FillDeletedBlock_(ptr, size));

    FLinearHeapBlock_* blk = FLinearHeapBlock_::BlockFromPtr(ptr);
    Assert_NoAssume(FLinearHeapBlock_::Contains(static_cast<FLinearHeapBlock_*>(_blocks), blk));

    const size_t off = blk->OffsetFromPtr(ptr);
    if (off + size == blk->Offset) {
        // reclaim memory
        blk->Offset = checked_cast<u32>(off);
    }
    else {
        Assert(size >= GLinearHeapMinBlockSizeForRecycling);
        FLinearHeapDeleted_::Delete(&_deleteds, ptr, size);
    }

#   if USE_CORE_MEMORYDOMAINS
    _trackingData.Deallocate(size / 16, 16);
#   endif

    if (0 == _numAllocs && _deleteds) {
        // reset allocator state if all blocks were released
        // will prevent the heap from degenerating in a list of deleted blocks
        FlushDeleteds_AssumeEmpty();
        Assert(nullptr == _deleteds);
    }

#endif
}
//----------------------------------------------------------------------------
void* FLinearHeap::Relocate_AssumeLast(void* ptr, size_t newSize, size_t oldSize) {
#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    Assert(ptr);

    auto& header = FLinearHeapHeader_::FromData(ptr);
    AssertRelease(&header == _blocks); // last block allocated

    return Relocate(ptr, newSize, header.SizeInBytes);

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
#if USE_CORE_MEMORYDOMAINS
        _trackingData.Deallocate(oldSize / 16, 16);
        _trackingData.Allocate(newSize / 16, 16);
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

#if !USE_CORE_FINAL_RELEASE
    DumpMemoryStats(L"FLinearHeap::ReleaseAll");
#endif

#if WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    Assert(nullptr == _deleteds);

    while (_blocks) {
        auto* blk = static_cast<FLinearHeapHeader_*>(_blocks);
        Release_AssumeLast(blk->data(), blk->SizeInBytes);
    }

    Assert_NoAssume(_trackingData.AllocationCount() == 0);
    Assert(nullptr == _blocks);

#else
    for (auto* blk = static_cast<FLinearHeapBlock_*>(_blocks); blk; )
        blk = FLinearHeapVMCache_::ReleaseBlock(blk);

    _numAllocs = 0;
    _blocks = nullptr;
    _deleteds = nullptr;
#   if USE_CORE_MEMORYDOMAINS
    _trackingData.ReleaseAll();
#   endif

#endif
}
//----------------------------------------------------------------------------
#if !USE_CORE_FINAL_RELEASE
void FLinearHeap::DumpMemoryStats(const wchar_t* title) {
#if defined(USE_DEBUG_LOGGER) && USE_CORE_MEMORYDOMAINS
    Assert(title);

    LOG(MemoryDomain, Debug, L"dumping memory statis for linear heap : {0}", MakeCStringView(title));

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

    LOG(MemoryDomain, Debug, L"linear heap <{0}> allocated {1} blocks using {2} pages, ie {3:f2} / {4:f2} allocated ({5}, max:{6})",
        _trackingData.Parent()
            ? MakeCStringView(_trackingData.Parent()->Name())
            : MakeCStringView(_trackingData.Name()),
        Fmt::CountOfElements(_trackingData.BlockCount()),
        totalBlockCount,
        Fmt::SizeInBytes(_trackingData.TotalSizeInBytes()),
        Fmt::SizeInBytes(totalSizeInBytes),
        Fmt::Percentage(_trackingData.TotalSizeInBytes(), totalSizeInBytes),
        Fmt::Percentage(_trackingData.MaxTotalSizeInBytes(), totalSizeInBytes));

#   if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
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

        LOG(MemoryDomain, Debug, L"linear heap <{0}> deleted [{1:5}] : {2:4} blocks ({3:10f3} total memory)",
            _trackingData.Parent()
                ? MakeCStringView(_trackingData.Parent()->Name())
                : MakeCStringView(_trackingData.Name()),
            blk->Size,
            Fmt::CountOfElements(blockCount),
            Fmt::SizeInBytes(sizeInBytes));

        maxBlockSize = Max(maxBlockSize, blk->Size);
        minBlockSize = Min(minBlockSize, blk->Size);
        totalBlockCount += blockCount;
        totalSizeInBytes += sizeInBytes;
    }

    LOG(MemoryDomain, Debug, L"linear heap <{0}> had {1} deleted blocks, total of {2} recyclable ({3} -> {4})",
        _trackingData.Parent()
            ? MakeCStringView(_trackingData.Parent()->Name())
            : MakeCStringView(_trackingData.Name()),
        totalBlockCount,
        Fmt::SizeInBytes(totalSizeInBytes),
        Fmt::SizeInBytes(minBlockSize),
        Fmt::SizeInBytes(maxBlockSize));
#   endif //!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC

#endif
}
#endif //!!USE_CORE_FINAL_RELEASE
//----------------------------------------------------------------------------
NO_INLINE void FLinearHeap::FlushDeleteds_AssumeEmpty() {
#if !WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
    Assert(0 == _numAllocs);
    Assert(_blocks);
    Assert(_deleteds);

#   if USE_CORE_MEMORYDOMAINS
    Assert(0 == _trackingData.AllocationCount());
#   endif

    auto* blk = static_cast<FLinearHeapBlock_*>(_blocks);

    // keep only one block alive
    for (FLinearHeapBlock_* spare = blk->Next; spare; )
        spare = FLinearHeapVMCache_::ReleaseBlock(spare);

    blk->Offset = 0;
    blk->Next = nullptr;
    _deleteds = nullptr;
#endif //!WITH_CORE_LINEARHEAP_FALLBACK_TO_MALLOC
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
