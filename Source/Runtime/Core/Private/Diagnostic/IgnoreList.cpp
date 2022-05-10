#include "stdafx.h"

#include "Diagnostic/IgnoreList.h"

#if USE_PPE_IGNORELIST

#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformHash.h"
#include "Memory/HashFunctions.h"
#include "Thread/ThreadContext.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static volatile int GIgnoreList_Available{ 0 };
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FIgnoreList::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared lib
}
//----------------------------------------------------------------------------
FIgnoreList::FIgnoreList() {
    VerifyRelease(FPlatformAtomics::Increment(&GIgnoreList_Available) == 1);
}
//----------------------------------------------------------------------------
FIgnoreList::~FIgnoreList() {
    VerifyRelease(FPlatformAtomics::Decrement(&GIgnoreList_Available) == 0);
}
//----------------------------------------------------------------------------
auto FIgnoreList::FIgnoreKey::AppendThread() NOEXCEPT -> FIgnoreKey& {
    const size_t threadSeed = std::hash<std::thread::id>{}(std::this_thread::get_id());
    return Append(MakeRawConstView(threadSeed));
}
//----------------------------------------------------------------------------
auto FIgnoreList::FIgnoreKey::Append(FRawMemoryConst key) NOEXCEPT -> FIgnoreKey& {
    const u128 digest = hash_128(key.data(), key.SizeInBytes(),
        hash_value(Fingerprint) );

    Fingerprint = {
        FPlatformHash::HashCombine64(Fingerprint.lo, digest.lo),
        FPlatformHash::HashCombine64(Fingerprint.hi, digest.hi)
    };

    return (*this);
}
//----------------------------------------------------------------------------
void FIgnoreList::Add(const FIgnoreKey& key) {
    _hits.LockExclusive()->insert({ key, 1 });
}
//----------------------------------------------------------------------------
auto FIgnoreList::Hit(const FIgnoreKey& key) -> FHitCount {
    const auto exclusive = _hits.LockExclusive();
    const auto [it, exist] = exclusive->insert({key, 0});
    return it->second++;
}
//----------------------------------------------------------------------------
bool FIgnoreList::Ignored(const FIgnoreKey& key) const NOEXCEPT {
    FHitCount hitCount;
    if (TryGetValue(*_hits.LockShared(), key, &hitCount)) {
        Assert_NoAssume(hitCount > 0);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
void FIgnoreList::Clear() {
    _hits.LockExclusive()->clear();
}
//----------------------------------------------------------------------------
bool FIgnoreList::Available() {
    return (FPlatformAtomics::Fetch(&GIgnoreList_Available) > 0);
}
//----------------------------------------------------------------------------
void FIgnoreList::AddIFP(const FIgnoreKey& key) {
    if (Likely(Available()))
        Get().Add(key);
}
//----------------------------------------------------------------------------
auto FIgnoreList::HitIFP(const FIgnoreKey& key) -> FHitCount {
    if (Likely(Available()))
        return Get().Hit(key);
    return 0;
}
//----------------------------------------------------------------------------
bool FIgnoreList::IgnoredIFP(const FIgnoreKey& key) NOEXCEPT {
    if (Likely(Available()))
        return Get().Ignored(key);
    return false;
}
//----------------------------------------------------------------------------
void FIgnoreList::ClearIFP() {
    if (Likely(Available()))
        return Get().Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_IGNORELIST
