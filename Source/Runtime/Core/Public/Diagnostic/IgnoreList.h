#pragma once

#include "Core_fwd.h"

#define USE_PPE_IGNORELIST (USE_PPE_ASSERT||USE_PPE_ASSERT_RELEASE)

#if USE_PPE_IGNORELIST

#include "Container/Hash.h"
#include "Container/Map.h"
#include "Meta/Singleton.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FIgnoreList : Meta::TSingleton<FIgnoreList> {
    friend class Meta::TSingleton<FIgnoreList>;
    using singleton_type = Meta::TSingleton<FIgnoreList>;
    PPE_CORE_API static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT; // for shared lib

    FIgnoreList();
public:
    ~FIgnoreList();

    using FHitCount = size_t;

    struct FIgnoreKey {
        u128 Fingerprint{ PPE_HASH_VALUE_SEED_64, PPE_HASH_VALUE_SEED_64 };

        PPE_CORE_API FIgnoreKey& AppendThread() NOEXCEPT; // seed the key with the thread id => will be specific to this thread
        PPE_CORE_API FIgnoreKey& Append(FRawMemoryConst key) NOEXCEPT;

        template <typename T>
        FIgnoreKey& operator <<(TMemoryView<T> key) NOEXCEPT { return Append(key.template Cast<const u8>()); }
        FIgnoreKey& operator <<(FRawMemoryConst key) NOEXCEPT { return Append(key); }

        bool operator ==(const FIgnoreKey& other) const NOEXCEPT {
            return (Fingerprint == other.Fingerprint);
        }
        bool operator !=(const FIgnoreKey& other) const NOEXCEPT {
            return (not operator ==(other));
        }

        bool operator < (const FIgnoreKey& other) const NOEXCEPT {
            return (Fingerprint < other.Fingerprint);
        }
        bool operator >=(const FIgnoreKey& other) const NOEXCEPT {
            return (not operator <(other));
        }

        friend hash_t hash_value(const FIgnoreKey& key) NOEXCEPT {
            return hash_tuple(key.Fingerprint.lo, key.Fingerprint.hi);
        }
        friend void swap(FIgnoreKey& lhs, FIgnoreKey& rhs) NOEXCEPT {
            swap(lhs.Fingerprint, rhs.Fingerprint);
        }
    };

    struct FIgnoreScope {
        PPE_CORE_API FIgnoreScope() NOEXCEPT;
        PPE_CORE_API ~FIgnoreScope();
    };

    PPE_CORE_API void Add(const FIgnoreKey& key);
    NODISCARD PPE_CORE_API FHitCount Hit(const FIgnoreKey& key); // return > 0 if ignored
    NODISCARD PPE_CORE_API bool Ignored(const FIgnoreKey& key) const NOEXCEPT;
    PPE_CORE_API void Clear();

    // the static variants won't fail if the singleton is not available
    NODISCARD PPE_CORE_API static bool Available();
    PPE_CORE_API static void AddIFP(const FIgnoreKey& key);
    NODISCARD PPE_CORE_API static FHitCount HitIFP(const FIgnoreKey& key); // return > 0 if ignored
    NODISCARD PPE_CORE_API static bool IgnoredIFP(const FIgnoreKey& key) NOEXCEPT;
    PPE_CORE_API static void ClearIFP();

public: // singleton API
    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    PPE_CORE_API static void Create();
    PPE_CORE_API static void Destroy();

private:
    using FHitMap = MAP(Diagnostic, FIgnoreKey, FHitCount);
    TThreadSafe<FHitMap, EThreadBarrier::CriticalSection> _hits;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_IGNORELIST
