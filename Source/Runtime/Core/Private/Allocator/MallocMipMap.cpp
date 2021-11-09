#include "stdafx.h"

#include "Allocator/MallocMipMap.h"

#include "Allocator/InitSegAllocator.h"
#include "Allocator/MallocBinned.h"
#include "Allocator/MipMapAllocator.h"

#include "Memory/MemoryDomain.h"

#if !USE_PPE_FINAL_RELEASE
#   include "Allocator/StackLocalAllocator.h"
#   include "IO/Format.h"
#   include "IO/FormatHelpers.h"
#   include "IO/TextWriter.h"
#endif
#if USE_PPE_MEMORYDOMAINS
#   include "Memory/MemoryTracking.h"
#endif

//reserved virtual size for mip maps (not everything is consumed systematically)

#define PPE_MIPMAPS_MEDIUM_GRANULARITY  (size_t(64)*1024) // x 32 = 2Mb per chunk
#define PPE_MIPMAPS_MEDIUM_RESERVEDSIZE (size_t(CODE3264(512, 2048)) * 1024 * 1024) // 512Mb/2Gb

#define PPE_MIPMAPS_LARGE_GRANULARITY  (size_t(2)*1024*1024) // x 32 = 64Mb per chunk
#define PPE_MIPMAPS_LARGE_RESERVEDSIZE (size_t(CODE3264(1024, 8192)) * 1024 * 1024) // 1/8Gb

#define PPE_MIPMAPS_USE_HIERARCHICAL_MIPS (0)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
using FMediumMipMaps_ = TMipMapAllocator2 < TMipmapCpuTraits<
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TAG(MediumHeap),
#endif
    PPE_MIPMAPS_MEDIUM_GRANULARITY
>>;
static FMediumMipMaps_& MediumMips_() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FMediumMipMaps_>, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
using FLargeMipMaps_ = TMipMapAllocator2 < TMipmapCpuTraits<
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TAG(LargeHeap),
#endif
    PPE_MIPMAPS_LARGE_GRANULARITY
>>;
static FLargeMipMaps_& LargeMips_() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FLargeMipMaps_>, GInstance);
    return GInstance;
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
template <typename _Traits>
static u32 FetchMipmapInfos_(FMallocMipMap::FMipmapInfo* pinfo, TMipMapAllocator2<_Traits>& mips) NOEXCEPT {
    size_t totalAllocationCount = 0;
    size_t totalSizeAllocated = 0;
    size_t totalSizeCommitted = 0;
    size_t totalSizeReserved = 0;

    u32 numPages = 0;
    mips.EachMipmapPage([&, pinfo](FMipmapPage* page) {
        totalSizeReserved += TMipMapAllocator2<_Traits>::PageSize;
        if (Likely(pinfo && numPages++ < pinfo->Pages.size())) {
            FMallocMipMap::FMipmapInfo::FPage& pageInfo = pinfo->Pages[numPages - 1];

            u32 pageLargestFreeBlock = 0;
            u32 pageTotalAvailableSize = 0;

            pageInfo.vAddress = page->vAddressSpace;
            pageInfo.NumBlocks = page->EachBlock(
                TMipMapAllocator2<_Traits>::MakePaging(),
                [&](u32 blk, u32 largestFreeBlockSize, u32 allocationMask, u32 committedMask) NOEXCEPT{
                    pageLargestFreeBlock = Max(pageLargestFreeBlock, largestFreeBlockSize);
                    pageTotalAvailableSize += FPlatformMaths::popcnt(~committedMask) * TMipMapAllocator2<_Traits>::MinAllocSize;

                    totalAllocationCount += FPlatformMaths::popcnt(allocationMask);
                    totalSizeCommitted += TMipMapAllocator2<_Traits>::BlockSize;
                    totalSizeAllocated += FPlatformMaths::popcnt(committedMask) * TMipMapAllocator2<_Traits>::MinAllocSize;

                    pageInfo.Blocks[blk] = { allocationMask, committedMask };
                });

            pageInfo.ExternalFragmentation = (pageTotalAvailableSize != 0
                ? (1.f - FPlatformMaths::Min(1.f, float(pageLargestFreeBlock) / pageTotalAvailableSize)) * 100
                : 0 );
        }
    });

    AssertRelease_NoAssume(totalSizeAllocated <= totalSizeCommitted);
    AssertRelease_NoAssume(totalSizeCommitted <= totalSizeReserved);

    if (pinfo) {
        pinfo->BlockSize = TMipMapAllocator2<_Traits>::BlockSize;
        pinfo->TotalAllocationCount = totalAllocationCount;
        pinfo->TotalSizeAllocated = totalSizeAllocated;
        pinfo->TotalSizeCommitted = totalSizeCommitted;
        pinfo->TotalSizeReserved = totalSizeReserved;

        return numPages;
    }
    else {
        CONSTEXPR u32 SafetySlack = 32;
        return (numPages + SafetySlack/* reserve more because of MT */);
    }
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void DumpMipsFragmentation_(
    FWTextWriter& oss,
    const FWStringView& name,
    u32 (*fFetchMips)(FMallocMipMap::FMipmapInfo*)) {
    // call with nullptr to know how much we must reserve
    const u32 reservedSize = fFetchMips(nullptr);

    // preallocate the space needed before fetching debug infos (can't allocate during fetch since this is our main allocator)
    STACKLOCAL_POD_ARRAY(FMallocMipMap::FMipmapInfo::FPage, reservedPages, reservedSize);
    FMallocMipMap::FMipmapInfo info;
    info.Pages = reservedPages;
    info.Pages = info.Pages.CutBefore(fFetchMips(&info));

    CONSTEXPR size_t width = 175;
    const auto hr = Fmt::Repeat(L'-', width);

    oss << FTextFormat::Float(FTextFormat::FixedFloat, 2)
        << L"  Report allocation internal fragmentation for <"
        << name
        << L"> : "
        << Fmt::CountOfElements(info.TotalAllocationCount)
        << L" , "
        << Fmt::SizeInBytes(info.BlockSize)
        << L" -> "
        << Fmt::SizeInBytes(info.TotalSizeAllocated)
        << L" / "
        << Fmt::SizeInBytes(info.TotalSizeCommitted)
        << L" / "
        << Fmt::SizeInBytes(info.TotalSizeReserved)
        << L" (" << Fmt::CountOfElements(info.Pages.size()) << L" pages)"
        << Eol << hr << Eol;

    CONSTEXPR const FWStringView AllocationTags = L"◇◈"; //L"◉○";// L"►◄"; // L"▬▭";// L"▪▫";// L"▮▯"; // L"▼▲";// L"○●";

    forrange(p, 0, info.Pages.size()) {
        const FMallocMipMap::FMipmapInfo::FPage& page = info.Pages[p];

        Format(oss, L"Page#{0} : {1} blocks committed -> {2}, external fragmentation = {3}",
            p, page.NumBlocks, Fmt::SizeInBytes(page.NumBlocks * info.BlockSize), Fmt::FPercentage{ page.ExternalFragmentation });
        const u8* const vAddr = reinterpret_cast<const u8*>(page.vAddress);

        size_t tag = size_t(-1);
        forrange(b, 0, size_t(32)) {
            if ((b % 4) == 0) {
                oss << Eol << Fmt::Pointer(vAddr + b * info.BlockSize) << L"   ";
            }
            if (b < page.NumBlocks) {
                const FMallocMipMap::FMipmapInfo::FPage::FBlock& block = page.Blocks[b];
                forrange(m, 0, u32(32)) {
                    if (block.AllocationMask & (u32(1) << m)) tag = (tag + 1) % AllocationTags.size();
                    oss.Put((block.CommittedMask & (u32(1) << m)) ? AllocationTags[tag] : L'▒');
                }
            }
            else {
                oss << Fmt::Repeat(L'░', 32);
            }
            oss << L' ';
        }

        oss << Eol;
    }
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const size_t FMallocMipMap::MediumMaxAllocSize = FMediumMipMaps_::MaxAllocSize;
const size_t FMallocMipMap::LargeMaxAllocSize = FLargeMipMaps_::MaxAllocSize;
const size_t FMallocMipMap::MipMaxAllocSize = FLargeMipMaps_::MaxAllocSize;
//----------------------------------------------------------------------------
void* FMallocMipMap::MediumAlloc(size_t sz, size_t alignment) {
    UNUSED(alignment);
    void* const newp = MediumMips_().Allocate(sz);
    Assert_NoAssume(!newp || Meta::IsAlignedPow2(alignment, newp));
    return newp;
}
//----------------------------------------------------------------------------
void* FMallocMipMap::MediumResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(newSize <= FMediumMipMaps_::MaxAllocSize);
    Assert_NoAssume(oldSize <= FMediumMipMaps_::MaxAllocSize);
    Assert_NoAssume(oldSize >= FMediumMipMaps_::MinAllocSize);
    Assert_NoAssume(MediumMips_().AllocationSize(ptr) == oldSize);

    void* const newp = MediumMips_().Resize(ptr, newSize);
    if (Likely(newp)) {
        Assert(newp == ptr);
        Assert_NoAssume(MediumMips_().AllocationSize(newp) == newSize);
        UNUSED(oldSize);
        return newp;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void FMallocMipMap::MediumFree(void* ptr) {
    Assert_NoAssume(AliasesToMediumMips(ptr));
    MediumMips_().Free(ptr);
}
//----------------------------------------------------------------------------
void FMallocMipMap::MediumTrim() {
    MediumMips_().ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocMipMap::AliasesToMediumMips(void* ptr) NOEXCEPT {
    return (MediumMips_().AliasingMipMap(ptr) != nullptr);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::MediumSnapSize(size_t sz) NOEXCEPT {
    return FMediumMipMaps_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::MediumRegionSize(void* ptr) NOEXCEPT {
    Assert_NoAssume(AliasesToMediumMips(ptr));
    return MediumMips_().AllocationSize(ptr);
}
//----------------------------------------------------------------------------
void* FMallocMipMap::LargeAlloc(size_t sz, size_t alignment) {
    UNUSED(alignment);
    void* const newp = LargeMips_().Allocate(sz);
    Assert_NoAssume(!newp || Meta::IsAlignedPow2(alignment, newp));
    return newp;
}
//----------------------------------------------------------------------------
void* FMallocMipMap::LargeResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT {
    Assert(ptr);
    Assert_NoAssume(newSize <= FLargeMipMaps_::MaxAllocSize);
    Assert_NoAssume(oldSize <= FLargeMipMaps_::MaxAllocSize);
    Assert_NoAssume(oldSize >= FLargeMipMaps_::MinAllocSize);
    Assert_NoAssume(LargeMips_().AllocationSize(ptr) == oldSize);

    void* const newp = LargeMips_().Resize(ptr, newSize);
    if (Likely(newp)) {
        Assert(newp == ptr);
        Assert_NoAssume(LargeMips_().AllocationSize(newp) == newSize);
        UNUSED(oldSize);
        return newp;
    }

    return nullptr;
}
//----------------------------------------------------------------------------
void FMallocMipMap::LargeFree(void* ptr) {
    Assert_NoAssume(AliasesToLargeMips(ptr));
    LargeMips_().Free(ptr);
}
//----------------------------------------------------------------------------
void FMallocMipMap::LargeTrim() {
    LargeMips_().ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocMipMap::AliasesToLargeMips(void* ptr) NOEXCEPT {
    return (LargeMips_().AliasingMipMap(ptr) != nullptr);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::LargeSnapSize(size_t sz) NOEXCEPT {
    return FLargeMipMaps_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::LargeRegionSize(void* ptr) NOEXCEPT {
    Assert_NoAssume(AliasesToLargeMips(ptr));
    return LargeMips_().AllocationSize(ptr);
}
//----------------------------------------------------------------------------
void* FMallocMipMap::MipAlloc(size_t sz, size_t alignment) {
    Assert_NoAssume(sz > FMallocBinned::MaxSmallBlockSize);

    if (Likely(sz <= FMediumMipMaps_::MaxAllocSize))
        return MediumAlloc(MediumSnapSize(sz), alignment);
    if (sz <= FLargeMipMaps_::MaxAllocSize)
        return LargeAlloc(LargeSnapSize(sz), alignment);

    return nullptr;
}
//----------------------------------------------------------------------------
void* FMallocMipMap::MipResize(void* ptr, size_t newSize, size_t oldSize) NOEXCEPT {
    Assert(ptr);
    Assert(newSize);
    Assert(oldSize);
    Assert(newSize != oldSize);
    Assert_NoAssume(newSize > FMallocBinned::MaxSmallBlockSize);
    Assert_NoAssume(oldSize > FMallocBinned::MaxSmallBlockSize);
    Assert_NoAssume(AliasesToMips(ptr));

    if (Likely((newSize <= FMediumMipMaps_::MaxAllocSize) & (oldSize <= FMediumMipMaps_::MaxAllocSize)))
        return MediumResize(ptr, MediumSnapSize(newSize), MediumSnapSize(oldSize));

    if ((newSize <= FLargeMipMaps_::MaxAllocSize) & (oldSize <= FLargeMipMaps_::MaxAllocSize) &
        (newSize > FMediumMipMaps_::MaxAllocSize) & (oldSize > FMediumMipMaps_::MaxAllocSize) )
        return LargeResize(ptr, LargeSnapSize(newSize), LargeSnapSize(oldSize));

    return nullptr; // can't resize without moving the data, but there might still be some space left
}
//----------------------------------------------------------------------------
void FMallocMipMap::MipFree(void* ptr) {
    if (Likely(AliasesToMediumMips(ptr)))
        MediumFree(ptr);
    else {
        Assert_NoAssume(AliasesToLargeMips(ptr));
        LargeFree(ptr);
    }
}
//----------------------------------------------------------------------------
void FMallocMipMap::MemoryTrim() {
    MediumMips_().ForceGarbageCollect();
    LargeMips_().ForceGarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocMipMap::AliasesToMips(void* ptr) NOEXCEPT {
    return (AliasesToMediumMips(ptr) || AliasesToLargeMips(ptr));
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::SnapSize(size_t sz) NOEXCEPT {
    if (Likely(sz <= FMediumMipMaps_::MaxAllocSize))
        return FMediumMipMaps_::SnapSize(sz);
    else
        return FLargeMipMaps_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::RegionSize(void* ptr) NOEXCEPT {
    const FMipmapPage* page = MediumMips_().AliasingMipMap(ptr);
    if (Likely(page))
        return page->AllocationSize(FMediumMipMaps_::MakePaging(), ptr);
    else {
        return LargeMips_().AllocationSize(ptr);
    }
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
u32 FMallocMipMap::FetchMediumMipsInfo(FMipmapInfo* pinfo) NOEXCEPT {
    return FetchMipmapInfos_(pinfo, MediumMips_());
}
u32 FMallocMipMap::FetchLargeMipsInfo(FMipmapInfo* pinfo) NOEXCEPT {
    return FetchMipmapInfos_(pinfo, LargeMips_());
}
void FMallocMipMap::DumpMemoryInfo(FWTextWriter& oss) {
    DumpMipsFragmentation_(oss, L"MediumMips", &FetchMediumMipsInfo);
    DumpMipsFragmentation_(oss, L"LargeMips", &FetchLargeMipsInfo);
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
