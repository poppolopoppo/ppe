#include "stdafx.h"

#include "Allocator/MallocBitmap.h"

#include "Allocator/BitmapHeap.h"
#include "Allocator/InitSegAllocator.h"
#include "Memory/MemoryDomain.h"

#if !USE_PPE_FINAL_RELEASE
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#   include "IO/TextWriter.h"
#endif
#if USE_PPE_MEMORYDOMAINS
#   include "Memory/MemoryTracking.h"
#endif

#define PPE_BITMAPHEAPS_MEDIUM_GRANULARITY  (size_t(32)*1024) // x 64 = 2Mb per chunk (div 2 for x86)
#define PPE_BITMAPHEAPS_LARGE_GRANULARITY  (size_t(1)*1024*1024) // x 64 = 64Mb per chunk (div 2 for x86)

#define PPE_BITMAPHEAPS_USE_HIERARCHICAL_MIPS (0)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
using FBitmapHeapMedium_ = TBitmapHeap< TBitmapCpuTraits<
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TAG(MediumHeap),
#endif
    PPE_BITMAPHEAPS_MEDIUM_GRANULARITY
>>;
static FBitmapHeapMedium_& BitmapHeapMedium_() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FBitmapHeapMedium_>, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
using FBitmapHeapLarge_ = TBitmapHeap< TBitmapCpuTraits<
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TAG(LargeHeap),
#endif
    PPE_BITMAPHEAPS_LARGE_GRANULARITY
>>;
static FBitmapHeapLarge_& BitmapHeapLarge_() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FBitmapHeapLarge_>, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
template <typename _Traits>
static void DumpHeapInfo_(FWTextWriter& oss, const FWStringView& name, const TBitmapHeap<_Traits>& heap) {
    // call with nullptr to know how much we must reserve
    const size_t numPagesReserved = (heap.DebugInfo(nullptr) + 16/* extra reserve for MT */);

    // preallocate the space needed before fetching debug infos (can't allocate during fetch since this is our main allocator)
    STACKLOCAL_POD_ARRAY(FBitmapPageInfo, reservedPages, numPagesReserved);
    FBitmapHeapInfo info;
    info.Pages = reservedPages;
    info.Pages = info.Pages.CutBefore(heap.DebugInfo(&info));

    CONSTEXPR size_t width = 175;
    const auto hr = Fmt::Repeat(L'-', width);

    oss << Eol << hr << Eol
        << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << L"Report allocation internal fragmentation for <"
        << name
        << L"> : "
        << FTextFormat::PadLeft(2, L' ')
        << Fmt::CountOfElements(info.Stats.NumAllocations)
        << L" , "
        << info.PagesPerBlock
        << L" x "
        << Fmt::SizeInBytes(info.Granularity)
        << L" -> "
        << Fmt::SizeInBytes(info.Stats.LargestFreeBlock)
        << L" / "
        << Fmt::SizeInBytes(info.Stats.TotalSizeAvailable)
        << L" + "
        << Fmt::SizeInBytes(info.Stats.TotalSizeAllocated)
        << L" = "
        << Fmt::SizeInBytes(info.Stats.TotalSizeCommitted
        )
        << L" (" << Fmt::CountOfElements(info.Pages.size()) << L" pages)"
        << Eol;

    CONSTEXPR const FWStringView AllocationTags = L"●○◉○"; // L"◇◈";// L"►◄"; // L"▬▭";// L"▪▫";// L"▮▯"; // L"▼▲";// L"◉○";

    forrange(p, 0, info.Pages.size()) {
        const FBitmapPageInfo& page = info.Pages[p];

        Format(oss, L"   Page#{0} : {1} allocations -> {2} / {3}, external fragmentation = {4}",
            p, page.Stats.NumAllocations,
            Fmt::SizeInBytes(page.Stats.LargestFreeBlock),
            Fmt::SizeInBytes(page.Stats.TotalSizeCommitted),
            Fmt::FPercentage{ 100.f - page.Stats.LargestFreeBlock * (100.f / page.Stats.TotalSizeAvailable) });

        oss << Eol << L"   " << Fmt::Pointer(page.vAddressSpace) << L"   ";

        size_t tag = size_t(0);
        forrange(b, 0, info.PagesPerBlock) {
            const u64 select = (u64(1) << b);
            oss.Put(not (page.Pages & select) ? AllocationTags[tag] : L'▒');
            tag = (tag + ((page.Sizes & select) ? 1 : 0)) % AllocationTags.size();
        }

        oss << L"  " << Fmt::SizeInBytes(page.Stats.TotalSizeAvailable) << Eol;
    }
}
#endif //#if !USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const size_t FMallocBitmap::Alignment = FBitmapHeapMedium_::MinAllocSize;
const size_t FMallocBitmap::MediumMaxAllocSize = FBitmapHeapMedium_::MaxAllocSize;
const size_t FMallocBitmap::LargeMaxAllocSize = FBitmapHeapLarge_::MaxAllocSize;
const size_t FMallocBitmap::MaxAllocSize = FBitmapHeapLarge_::MaxAllocSize;
//----------------------------------------------------------------------------
void* FMallocBitmap::MediumAlloc(size_t sz, size_t alignment) {
    UNUSED(alignment);
    void* const newp = BitmapHeapMedium_().Allocate(sz);
    Assert_NoAssume(!newp || Meta::IsAligned(alignment, newp));
    Assert_NoAssume(!newp || Meta::IsAligned(FBitmapHeapMedium_::Granularity, newp));
    return newp;
}
//----------------------------------------------------------------------------
void* FMallocBitmap::MediumResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(newSize <= FBitmapHeapMedium_::MaxAllocSize);
    Assert_NoAssume(oldSize <= FBitmapHeapMedium_::MaxAllocSize);
    Assert_NoAssume(oldSize >= FBitmapHeapMedium_::MinAllocSize);
    Assert_NoAssume(BitmapHeapMedium_().AllocationSize(ptr) == oldSize);

    void* const newp = BitmapHeapMedium_().Resize(ptr, newSize);
    if (Likely(newp)) {
        Assert(newp == ptr);
        Assert_NoAssume(BitmapHeapMedium_().AllocationSize(newp) == newSize);
        UNUSED(oldSize);
        return newp;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void FMallocBitmap::MediumFree(void* ptr) {
    Assert_NoAssume(AliasesToMediumHeap(ptr));
    BitmapHeapMedium_().Free(ptr);
}
//----------------------------------------------------------------------------
void FMallocBitmap::MediumTrim() {
    BitmapHeapMedium_().ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocBitmap::AliasesToMediumHeap(void* ptr) NOEXCEPT {
    return (BitmapHeapMedium_().Aliases(ptr) != nullptr);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::MediumSnapSize(size_t sz) NOEXCEPT {
    return FBitmapHeapMedium_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::MediumRegionSize(void* ptr) NOEXCEPT {
    Assert_NoAssume(AliasesToMediumHeap(ptr));
    return BitmapHeapMedium_().AllocationSize(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBitmap::LargeAlloc(size_t sz, size_t alignment) {
    UNUSED(alignment);
    void* const newp = BitmapHeapLarge_().Allocate(sz);
    Assert_NoAssume(!newp || Meta::IsAligned(alignment, newp));
    Assert_NoAssume(!newp || Meta::IsAligned(FBitmapHeapLarge_::Granularity, newp));
    return newp;
}
//----------------------------------------------------------------------------
void* FMallocBitmap::LargeResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(newSize <= FBitmapHeapLarge_::MaxAllocSize);
    Assert_NoAssume(oldSize <= FBitmapHeapLarge_::MaxAllocSize);
    Assert_NoAssume(oldSize >= FBitmapHeapLarge_::MinAllocSize);
    Assert_NoAssume(BitmapHeapLarge_().AllocationSize(ptr) == oldSize);

    void* const newp = BitmapHeapLarge_().Resize(ptr, newSize);
    if (Likely(newp)) {
        Assert(newp == ptr);
        Assert_NoAssume(BitmapHeapLarge_().AllocationSize(newp) == newSize);
        UNUSED(oldSize);
        return newp;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void FMallocBitmap::LargeFree(void* ptr) {
    Assert_NoAssume(AliasesToLargeHeap(ptr));
    BitmapHeapLarge_().Free(ptr);
}
//----------------------------------------------------------------------------
void FMallocBitmap::LargeTrim() {
    BitmapHeapLarge_().ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocBitmap::AliasesToLargeHeap(void* ptr) NOEXCEPT {
    return (BitmapHeapLarge_().Aliases(ptr) != nullptr);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::LargeSnapSize(size_t sz) NOEXCEPT {
    return FBitmapHeapLarge_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::LargeRegionSize(void* ptr) NOEXCEPT {
    Assert_NoAssume(AliasesToLargeHeap(ptr));
    return BitmapHeapLarge_().AllocationSize(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBitmap::HeapAlloc(size_t sz, size_t alignment) {
    if (Likely(sz <= FBitmapHeapMedium_::MaxAllocSize))
        return MediumAlloc(MediumSnapSize(sz), alignment);
    if (sz <= FBitmapHeapLarge_::MaxAllocSize)
        return LargeAlloc(LargeSnapSize(sz), alignment);

    return nullptr;
}
//----------------------------------------------------------------------------
void* FMallocBitmap::HeapResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT {
    Assert(ptr);
    Assert(newSize);
    Assert(oldSize);
    Assert(newSize != oldSize);
    Assert_NoAssume(AliasesToHeaps(ptr));

    if (Likely((newSize <= FBitmapHeapMedium_::MaxAllocSize) & (oldSize <= FBitmapHeapMedium_::MaxAllocSize)))
        return MediumResize(ptr, MediumSnapSize(newSize), MediumSnapSize(oldSize));

    if ((newSize <= FBitmapHeapLarge_::MaxAllocSize) & (oldSize <= FBitmapHeapLarge_::MaxAllocSize) &
        (newSize >  FBitmapHeapMedium_::MaxAllocSize) & (oldSize >  FBitmapHeapMedium_::MaxAllocSize) )
        return LargeResize(ptr, LargeSnapSize(newSize), LargeSnapSize(oldSize));

    return nullptr; // can't resize without moving the data, but there might still be some space left
}
//----------------------------------------------------------------------------
void FMallocBitmap::HeapFree(void* ptr) {
    if (Likely(AliasesToMediumHeap(ptr)))
        MediumFree(ptr);
    else {
        Assert_NoAssume(AliasesToLargeHeap(ptr));
        LargeFree(ptr);
    }
}
//----------------------------------------------------------------------------
void FMallocBitmap::MemoryTrim() {
    BitmapHeapMedium_().ForceGarbageCollect();
    BitmapHeapLarge_().ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocBitmap::AliasesToHeaps(void* ptr) NOEXCEPT {
    return (AliasesToMediumHeap(ptr) || AliasesToLargeHeap(ptr));
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::SnapSize(size_t sz) NOEXCEPT {
    if (Likely(sz <= FBitmapHeapMedium_::MaxAllocSize))
        return FBitmapHeapMedium_::SnapSize(sz);
    else
        return FBitmapHeapLarge_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::RegionSize(void* ptr) NOEXCEPT {
    const FBitmapHeapMedium_::page_type* page = BitmapHeapMedium_().Aliases(ptr);
    if (Likely(page))
        return page->RegionSize(ptr);
    else {
        return BitmapHeapLarge_().AllocationSize(ptr);
    }
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocBitmap::DumpMediumHeapInfo(FWTextWriter& oss) NOEXCEPT {
    DumpHeapInfo_(oss, L"BitmapHeapMedium", BitmapHeapMedium_());
}
void FMallocBitmap::DumpLargeHeapInfo(FWTextWriter& oss) NOEXCEPT {
    DumpHeapInfo_(oss, L"BitmapHeapLarge", BitmapHeapLarge_());
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
