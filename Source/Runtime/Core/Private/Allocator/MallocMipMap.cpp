#include "stdafx.h"

#include "Allocator/MallocMipMap.h"

#include "Allocator/InitSegAllocator.h"
#include "Allocator/MipMapAllocator.h"

//reserved virtual size for mip maps (not everything is consumed systematically)

#define PPE_MIPMAPS_MEDIUM_GRANULARITY  (size_t(64)*1024) // x 32 = 2Mb per chunk
#define PPE_MIPMAPS_MEDIUM_RESERVEDSIZE (size_t(CODE3264(512, 2048)) * 1024 * 1024) // 512Mb/2Gb

#define PPE_MIPMAPS_LARGE_GRANULARITY  (size_t(2)*1024*1024) // x 32 = 64Mb per chunk
#define PPE_MIPMAPS_LARGE_RESERVEDSIZE (size_t(CODE3264(1024, 8192)) * 1024 * 1024) // 1/8Gb

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
using FMediumMipMaps_ = TMipMapAllocator<TCPUMipMapVMemTraits<
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TAG(MediumMipMaps),
#endif
    PPE_MIPMAPS_MEDIUM_GRANULARITY,
    PPE_MIPMAPS_MEDIUM_RESERVEDSIZE
>>;
static FMediumMipMaps_& MediumMips_() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FMediumMipMaps_>, GMips);
    return GMips;
}
//----------------------------------------------------------------------------
using FLargeMipMaps_ = TMipMapAllocator<TCPUMipMapVMemTraits<
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TAG(LargeMipMaps),
#endif
    PPE_MIPMAPS_LARGE_GRANULARITY,
    PPE_MIPMAPS_LARGE_RESERVEDSIZE
>>;
static FLargeMipMaps_& LargeMips_() NOEXCEPT {
    ONE_TIME_DEFAULT_INITIALIZE(TInitSegAlloc<FLargeMipMaps_>, GMips);
    return GMips;
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
template <typename _MipMaps>
static void FetchMipsForDebug_(
    _MipMaps& mips,
    void** vspace,
    size_t* numCommited,
    size_t* numReserved,
    size_t* mipSizeInBytes,
    TMemoryView<const u32>* mipMasks ) {
    Assert(vspace);
    Assert(numCommited);
    Assert(numReserved);
    Assert(mipSizeInBytes);
    Assert(mipMasks);

    // #TODO : this is *really* ugly, even for debug code :/
    constexpr size_t NumReserved = _MipMaps::ReservedSize / _MipMaps::TopMipSize;
    static u32 GMips[NumReserved];

    const size_t n = mips.EachCommitedMipMap([](size_t mipIndex, u32 mipMask) {
        GMips[mipIndex] = mipMask;
    });

    *vspace = mips.VSpace();
    *numCommited = n;
    *numReserved = NumReserved;
    *mipSizeInBytes = _MipMaps::TopMipSize;
    *mipMasks = MakeConstView(GMips).CutBefore(n);
}
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const size_t FMallocMipMap::MediumTopMipSize = FMediumMipMaps_::TopMipSize;
const size_t FMallocMipMap::LargeTopMipSize = FLargeMipMaps_::TopMipSize;
//----------------------------------------------------------------------------
void* FMallocMipMap::MediumAlloc(size_t sz, size_t alignment) {
    ONE_TIME_INITIALIZE_THREAD_LOCAL(size_t, GHintTLS, 0);
    void* const newp = MediumMips_().Allocate(sz, &GHintTLS);
#if USE_PPE_MEMORYDOMAINS
    if (newp) {
        Assert(Meta::IsAligned(alignment, newp));
        MEMORYDOMAIN_TRACKING_DATA(MediumMipMaps).AllocateUser(MediumMips_().AllocationSize(newp));
    }
#endif
    return newp;
}
//----------------------------------------------------------------------------
void FMallocMipMap::MediumFree(void* ptr) {
    Assert(AliasesToMediumMips(ptr));
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TRACKING_DATA(MediumMipMaps).DeallocateUser(MediumMips_().AllocationSize(ptr));
#endif
    MediumMips_().Free(ptr);
}
//----------------------------------------------------------------------------
void FMallocMipMap::MediumTrim() {
    MediumMips_().GarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocMipMap::AliasesToMediumMips(void* ptr) NOEXCEPT {
    return MediumMips_().AliasesToMipMaps(ptr);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::MediumSnapSize(size_t sz) NOEXCEPT {
    return FMediumMipMaps_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::MediumRegionSize(void* ptr) NOEXCEPT {
    Assert(MediumMips_().AliasesToMipMaps(ptr));
    return MediumMips_().AllocationSize(ptr);
}
//----------------------------------------------------------------------------
void* FMallocMipMap::LargeAlloc(size_t sz, size_t alignment) {
    ONE_TIME_INITIALIZE_THREAD_LOCAL(size_t, GHintTLS, 0);
    void* const newp = LargeMips_().Allocate(sz, &GHintTLS);
#if USE_PPE_MEMORYDOMAINS
    if (newp) {
        Assert(Meta::IsAligned(alignment, newp));
        MEMORYDOMAIN_TRACKING_DATA(LargeMipMaps).AllocateUser(LargeMips_().AllocationSize(newp));
    }
#endif
    return newp;
}
//----------------------------------------------------------------------------
void FMallocMipMap::LargeFree(void* ptr) {
    Assert(AliasesToLargeMips(ptr));
#if USE_PPE_MEMORYDOMAINS
    MEMORYDOMAIN_TRACKING_DATA(LargeMipMaps).DeallocateUser(LargeMips_().AllocationSize(ptr));
#endif
    LargeMips_().Free(ptr);
}
//----------------------------------------------------------------------------
void FMallocMipMap::LargeTrim() {
    LargeMips_().GarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocMipMap::AliasesToLargeMips(void* ptr) NOEXCEPT {
    return LargeMips_().AliasesToMipMaps(ptr);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::LargeSnapSize(size_t sz) NOEXCEPT {
    return FLargeMipMaps_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::LargeRegionSize(void* ptr) NOEXCEPT {
    Assert(LargeMips_().AliasesToMipMaps(ptr));
    return LargeMips_().AllocationSize(ptr);
}
//----------------------------------------------------------------------------
void* FMallocMipMap::MipAlloc(size_t sz, size_t alignment) {
    if (sz <= FMediumMipMaps_::TopMipSize)
        return MediumAlloc(sz, alignment);
    if (sz <= FMediumMipMaps_::TopMipSize)
        return LargeAlloc(sz, alignment);

    return nullptr;
}
//----------------------------------------------------------------------------
void FMallocMipMap::MipFree(void* ptr) {
    if (MediumMips_().AliasesToMipMaps(ptr)) {
        MediumFree(ptr);
    }
    else {
        Assert(LargeMips_().AliasesToMipMaps(ptr));
        LargeFree(ptr);
    }
}
//----------------------------------------------------------------------------
void FMallocMipMap::MemoryTrim() {
    MediumMips_().GarbageCollect();
    LargeMips_().GarbageCollect();
}
//----------------------------------------------------------------------------
bool FMallocMipMap::AliasesToMips(void* ptr) NOEXCEPT {
    return AliasesToMediumMips(ptr) || AliasesToLargeMips(ptr);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::SnapSize(size_t sz) NOEXCEPT {
    if (sz < FMediumMipMaps_::TopMipSize)
        return FMediumMipMaps_::SnapSize(sz);
    else
        return FLargeMipMaps_::SnapSize(sz);
}
//----------------------------------------------------------------------------
size_t FMallocMipMap::RegionSize(void* ptr) NOEXCEPT {
    if (MediumMips_().AliasesToMipMaps(ptr))
        return MediumMips_().AllocationSize(ptr);
    else {
        Assert(LargeMips_().AliasesToMipMaps(ptr));
        return LargeMips_().AllocationSize(ptr);
    }
}
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocMipMap::MediumMips(
    void** vspace,
    size_t* numCommited,
    size_t* numReserved,
    size_t* mipSizeInBytes,
    TMemoryView<const u32>* mipMasks ) {
    FetchMipsForDebug_(MediumMips_(), vspace, numCommited, numReserved, mipSizeInBytes, mipMasks);
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
#if !USE_PPE_FINAL_RELEASE
void FMallocMipMap::LargeMips(
    void** vspace,
    size_t* numCommited,
    size_t* numReserved,
    size_t* mipSizeInBytes,
    TMemoryView<const u32>* mipMasks ) {
    FetchMipsForDebug_(LargeMips_(), vspace, numCommited, numReserved, mipSizeInBytes, mipMasks);
}
#endif //!USE_PPE_FINAL_RELEASE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
