// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Allocator/MallocBitmap.h"

#include "Allocator/BitmapHeap.h"
#include "Allocator/InitSegAllocator.h"
#include "Memory/MemoryDomain.h"

#if !USE_PPE_FINAL_RELEASE
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#   include "IO/StringView.h"
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
    // packed together to avoid destruction order issues due to FPageAllocator dependency
    FBitmapHeapMedium_ Medium;
    FBitmapHeapLarge_ Large;

    static FBitmapHeaps_& Get() NOEXCEPT {
        ONE_TIME_INITIALIZE(TInitSegAlloc<FBitmapHeaps_>, GInstance, 3000);
        return GInstance;
    }
};
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
template <typename _Char, typename _Traits>
static void DumpHeapInfo_(TBasicTextWriter<_Char>& oss, const TBasicStringView<_Char>& name, const TBitmapHeap<_Traits>& heap) {
    // call with nullptr to know how much we must reserve
    const size_t numPagesReserved = (heap.DebugInfo(nullptr) + 16/* extra reserve for MT */);

    // preallocate the space needed before fetching debug infos (can't allocate during fetch since this is our main allocator)
    STACKLOCAL_POD_ARRAY(FBitmapPageInfo, reservedPages, numPagesReserved);
    FBitmapHeapInfo info;
    info.Pages = reservedPages;
    info.Pages = info.Pages.CutBefore(heap.DebugInfo(&info));

    CONSTEXPR size_t width = 175;
    const auto hr = Fmt::Repeat('-', width);

    oss << Eol << hr << Eol
        << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << STRING_LITERAL(_Char, "Report allocation internal fragmentation for <")
        << name
        << STRING_LITERAL(_Char, "> : ")
        << FTextFormat::PadLeft(2, STRING_LITERAL(_Char, ' '))
        << Fmt::CountOfElements(info.Stats.NumAllocations)
        << STRING_LITERAL(_Char, " , ")
        << info.PagesPerBlock
        << STRING_LITERAL(_Char, " x ")
        << Fmt::SizeInBytes(info.Granularity)
        << STRING_LITERAL(_Char, " -> ")
        << Fmt::SizeInBytes(info.Stats.LargestFreeBlock)
        << STRING_LITERAL(_Char, " / ")
        << Fmt::SizeInBytes(info.Stats.TotalSizeAvailable)
        << STRING_LITERAL(_Char, " + ")
        << Fmt::SizeInBytes(info.Stats.TotalSizeAllocated)
        << STRING_LITERAL(_Char, " = ")
        << Fmt::SizeInBytes(info.Stats.TotalSizeCommitted
        )
        << STRING_LITERAL(_Char, " (") << Fmt::CountOfElements(info.Pages.size()) << STRING_LITERAL(_Char, " pages)")
        << Eol;

    ////CONSTEXPR const wchar_t AllocationTags[2] = { L'▓', L'█' }; /* {
    //    STRING_LITERAL(_Char, "\xE2\x96\x92"),
    //    STRING_LITERAL(_Char, "\xE2\x96\x91"),
    //};*/ // "░▒▓█"
    //    // L"❶❷❸❹❺❻❼❽❾";
    //    // L"▬";// L"●◉"; // L"◇◈";// L"►◄"; // L"▪▫";// L"▮▯"; // L"▼▲";// L"◉○";

    forrange(p, 0, info.Pages.size()) {
        const FBitmapPageInfo& page = info.Pages[p];

        oss << STRING_LITERAL(_Char, "   ")
            << Fmt::Pointer(page.vAddressSpace)
            << STRING_LITERAL(_Char, "   ");

        char tag = 0;
        forrange(b, 0, info.PagesPerBlock) {
            const u64 select = (1_u64 << b);
            oss.Put((page.Pages & select) ? '_' : '0' + tag);
            tag = (tag + ((page.Sizes & select) ? 1 : 0)) % 10;
        }

        Format(oss, STRING_LITERAL(_Char, " #{0:#2}: {1:4} allocs, {2:10f2}/{3:10f2}/{4:10f2}, fragmentation: {5}"),
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
    Assert_NoAssume(FBitmapHeaps_::Get().Medium.AllocationSize(newp) == sz);
    return newp;
}
//----------------------------------------------------------------------------
void* FMallocBitmap::MediumReallocIFP(void* ptr, size_t sz) {
    return FBitmapHeaps_::Get().Medium.Resize(ptr, sz);
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
void* FMallocBitmap::LargeReallocIFP(void* ptr, size_t sz) {
    return FBitmapHeaps_::Get().Large.Resize(ptr, sz);
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
FAllocatorBlock FMallocBitmap::HeapAlloc(size_t sz, size_t alignment) {
    if (Likely(sz <= FBitmapHeapMedium_::MaxAllocSize)) {
        sz = MediumSnapSize(sz);
        return FAllocatorBlock(MediumAlloc(sz, alignment), sz);
    }
    if (sz <= FBitmapHeapLarge_::MaxAllocSize) {
        sz = LargeSnapSize(sz);
        return FAllocatorBlock(LargeAlloc(sz, alignment), sz);
    }

    return Default;
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
size_t FMallocBitmap::DumpMediumHeapInfo(FBitmapHeapInfo* pInfos) NOEXCEPT {
    return FBitmapHeaps_::Get().Medium.DebugInfo(pInfos);
}
size_t FMallocBitmap::DumpLargeHeapInfo(FBitmapHeapInfo* pInfos) NOEXCEPT {
    return FBitmapHeaps_::Get().Large.DebugInfo(pInfos);
}
void FMallocBitmap::DumpMediumHeapInfo(FTextWriter& oss) NOEXCEPT {
    DumpHeapInfo_(oss, MakeStringView("BitmapHeapMedium"), FBitmapHeaps_::Get().Medium);
}
void FMallocBitmap::DumpLargeHeapInfo(FTextWriter& oss) NOEXCEPT {
    DumpHeapInfo_(oss, MakeStringView("BitmapHeapLarge"), FBitmapHeaps_::Get().Large);
}
void FMallocBitmap::DumpMediumHeapInfo(FWTextWriter& oss) NOEXCEPT {
    DumpHeapInfo_(oss, MakeStringView(L"BitmapHeapMedium"), FBitmapHeaps_::Get().Medium);
}
void FMallocBitmap::DumpLargeHeapInfo(FWTextWriter& oss) NOEXCEPT {
    DumpHeapInfo_(oss, MakeStringView(L"BitmapHeapLarge"), FBitmapHeaps_::Get().Large);
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
