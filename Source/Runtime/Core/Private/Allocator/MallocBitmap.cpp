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

#define PPE_BITMAPHEAPS_MEDIUM_GRANULARITY  (32_KiB)    // x 64 = 2Mb per chunk (div 2 for x86)
#define PPE_BITMAPHEAPS_LARGE_GRANULARITY   (1_MiB)     // x 64 = 64Mb per chunk (div 2 for x86)

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
using FBitmapHeapLarge_ = TBitmapHeap< TBitmapCpuTraits<
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TAG(LargeHeap),
#endif
    PPE_BITMAPHEAPS_LARGE_GRANULARITY
    >>;
struct FBitmapHeaps_ {
    // packed together to avoid destruction order issues due to FSystemPageAllocator dependency
    FBitmapHeapMedium_ Medium;
    FBitmapHeapLarge_ Large;

    static FBitmapHeaps_& Get() NOEXCEPT {
        ONE_TIME_INITIALIZE(TInitSegAlloc<FBitmapHeaps_>, GInstance, 3000);
        return GInstance;
    }
};
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

    CONSTEXPR const FWStringView AllocationTags = L"█▓"; // L"░▒▓█"
        // L"❶❷❸❹❺❻❼❽❾";
        // L"▬";// L"●◉"; // L"◇◈";// L"►◄"; // L"▪▫";// L"▮▯"; // L"▼▲";// L"◉○";

    forrange(p, 0, info.Pages.size()) {
        const FBitmapPageInfo& page = info.Pages[p];


        oss << L"   " << Fmt::Pointer(page.vAddressSpace) << L"   ";

        size_t tag = size_t(0);
        forrange(b, 0, info.PagesPerBlock) {
            const u64 select = (u64(1) << b);
            oss.Put(not (page.Pages & select) ? AllocationTags[tag] : /*L'▒'*/L'░');
            tag = (tag + ((page.Sizes & select) ? 1 : 0)) % AllocationTags.size();
        }

        Format(oss, L" #{0:#2}: {1:4} allocs, {2:10f2}/{3:10f2}/{4:10f2}, fragmentation: {5}",
            p,
            Fmt::CountOfElements(page.Stats.NumAllocations),
            Fmt::SizeInBytes(page.Stats.LargestFreeBlock),
            Fmt::SizeInBytes(page.Stats.TotalSizeAvailable),
            Fmt::SizeInBytes(page.Stats.TotalSizeAllocated),
            Fmt::FPercentage{ page.Stats.TotalSizeAvailable
                ? 100.f - static_cast<float>(page.Stats.LargestFreeBlock) * (100.f / static_cast<float>(page.Stats.TotalSizeAvailable))
                : 0.0f });

        oss << Eol;
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
    Unused(alignment);
    void* const newp = FBitmapHeaps_::Get().Medium.Allocate(sz);
    Assert_NoAssume(!newp || Meta::IsAlignedPow2(alignment, newp));
    Assert_NoAssume(!newp || Meta::IsAlignedPow2(FBitmapHeapMedium_::Granularity, newp));
    return newp;
}
//----------------------------------------------------------------------------
void* FMallocBitmap::MediumResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(newSize <= FBitmapHeapMedium_::MaxAllocSize);
    Assert_NoAssume(oldSize <= FBitmapHeapMedium_::MaxAllocSize);
    Assert_NoAssume(oldSize >= FBitmapHeapMedium_::MinAllocSize);
    Assert_NoAssume(FBitmapHeaps_::Get().Medium.AllocationSize(ptr) == oldSize);

    void* const newp = FBitmapHeaps_::Get().Medium.Resize(ptr, newSize);
    if (Likely(newp)) {
        Assert(newp == ptr);
        Assert_NoAssume(FBitmapHeaps_::Get().Medium.AllocationSize(newp) == newSize);
        Unused(oldSize);
        return newp;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void FMallocBitmap::MediumFree(void* ptr) {
    Assert_NoAssume(AliasesToMediumHeap(ptr));
    FBitmapHeaps_::Get().Medium.Free(ptr);
}
//----------------------------------------------------------------------------
void FMallocBitmap::MediumTrim() {
    FBitmapHeaps_::Get().Medium.ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocBitmap::AliasesToMediumHeap(void* ptr) NOEXCEPT {
    return (FBitmapHeaps_::Get().Medium.Aliases(ptr) != nullptr);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::MediumSnapSize(size_t sz) NOEXCEPT {
    return FBitmapHeapMedium_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::MediumRegionSize(void* ptr) NOEXCEPT {
    Assert_NoAssume(AliasesToMediumHeap(ptr));
    return FBitmapHeaps_::Get().Medium.AllocationSize(ptr);
}
//----------------------------------------------------------------------------
void* FMallocBitmap::LargeAlloc(size_t sz, size_t alignment) {
    Unused(alignment);
    void* const newp = FBitmapHeaps_::Get().Large.Allocate(sz);
    Assert_NoAssume(!newp || Meta::IsAlignedPow2(alignment, newp));
    Assert_NoAssume(!newp || Meta::IsAlignedPow2(FBitmapHeapLarge_::Granularity, newp));
    return newp;
}
//----------------------------------------------------------------------------
void* FMallocBitmap::LargeResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(newSize <= FBitmapHeapLarge_::MaxAllocSize);
    Assert_NoAssume(oldSize <= FBitmapHeapLarge_::MaxAllocSize);
    Assert_NoAssume(oldSize >= FBitmapHeapLarge_::MinAllocSize);
    Assert_NoAssume(FBitmapHeaps_::Get().Large.AllocationSize(ptr) == oldSize);

    void* const newp = FBitmapHeaps_::Get().Large.Resize(ptr, newSize);
    if (Likely(newp)) {
        Assert(newp == ptr);
        Assert_NoAssume(FBitmapHeaps_::Get().Large.AllocationSize(newp) == newSize);
        Unused(oldSize);
        return newp;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void FMallocBitmap::LargeFree(void* ptr) {
    Assert_NoAssume(AliasesToLargeHeap(ptr));
    FBitmapHeaps_::Get().Large.Free(ptr);
}
//----------------------------------------------------------------------------
void FMallocBitmap::LargeTrim() {
    FBitmapHeaps_::Get().Large.ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocBitmap::AliasesToLargeHeap(void* ptr) NOEXCEPT {
    return (FBitmapHeaps_::Get().Large.Aliases(ptr) != nullptr);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::LargeSnapSize(size_t sz) NOEXCEPT {
    return FBitmapHeapLarge_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::LargeRegionSize(void* ptr) NOEXCEPT {
    Assert_NoAssume(AliasesToLargeHeap(ptr));
    return FBitmapHeaps_::Get().Large.AllocationSize(ptr);
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
bool FMallocBitmap::HeapFree_ReturnIfAliases(void* ptr) {
    auto& mediumHeap = FBitmapHeaps_::Get().Medium;
    if (auto* page = mediumHeap.Aliases(ptr))
        return mediumHeap.Free(ptr, page), true;

    auto& largeHeap = FBitmapHeaps_::Get().Large;
    if (auto* page = largeHeap.Aliases(ptr))
        return largeHeap.Free(ptr, page), true;

    return false;
}
//----------------------------------------------------------------------------
void FMallocBitmap::MemoryTrim() {
    FBitmapHeaps_::Get().Medium.ForceGarbageCollect();
    FBitmapHeaps_::Get().Large.ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocBitmap::AliasesToHeaps(void* ptr) NOEXCEPT {
    Assert(ptr);

    return (AliasesToMediumHeap(ptr) || AliasesToLargeHeap(ptr));
}
//----------------------------------------------------------------------------
size_t FMallocBitmap::SnapSize(size_t sz) NOEXCEPT {
    Assert(sz);

    if (Likely(sz <= FBitmapHeapMedium_::MaxAllocSize))
        return FBitmapHeapMedium_::SnapSize(sz);
    else
        return FBitmapHeapLarge_::SnapSize(sz);
}
//----------------------------------------------------------------------------
bool FMallocBitmap::RegionSize_ReturnIfAliases(size_t* pSizeInBytes, void* ptr) NOEXCEPT {
    Assert(pSizeInBytes);

    if (auto* page = FBitmapHeaps_::Get().Medium.Aliases(ptr))
        return *pSizeInBytes = page->RegionSize(ptr), true;

    if (auto* page = FBitmapHeaps_::Get().Large.Aliases(ptr))
        return *pSizeInBytes = page->RegionSize(ptr), true;

    return false;
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocBitmap::DumpMediumHeapInfo(FWTextWriter& oss) NOEXCEPT {
    DumpHeapInfo_(oss, L"BitmapHeapMedium", FBitmapHeaps_::Get().Medium);
}
void FMallocBitmap::DumpLargeHeapInfo(FWTextWriter& oss) NOEXCEPT {
    DumpHeapInfo_(oss, L"BitmapHeapLarge", FBitmapHeaps_::Get().Large);
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
