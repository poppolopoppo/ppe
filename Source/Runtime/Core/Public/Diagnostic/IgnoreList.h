#pragma once

#include "Core_fwd.h"

#define USE_PPE_IGNORELIST (USE_PPE_ASSERT||USE_PPE_ASSERT_RELEASE)

#if USE_PPE_IGNORELIST

#include "Container/Map.h"
#include "Meta/Singleton.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FIgnoreList : Meta::TSingleton<FIgnoreList> {
    friend class Meta::TSingleton<FIgnoreList>;
    using singleton_type = Meta::TSingleton<FIgnoreList>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT; // for shared lib

    FIgnoreList();
public:
    ~FIgnoreList();

    using FHitCount = size_t;

    struct FIgnoreKey {
        u128 Fingerprint{ PPE_HASH_VALUE_SEED_64, PPE_HASH_VALUE_SEED_64 };

        FIgnoreKey& AppendThread() NOEXCEPT; // seed the key with the thread id => will be specific to this thread
        FIgnoreKey& Append(FRawMemoryConst key) NOEXCEPT;

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

    void Add(const FIgnoreKey& key);
    FHitCount Hit(const FIgnoreKey& key); // return > 0 if ignored
    bool Ignored(const FIgnoreKey& key) const NOEXCEPT;
    void Clear();

    // the static variants won't fail if the singleton is not available
    static bool Available();
    static void AddIFP(const FIgnoreKey& key);
    static FHitCount HitIFP(const FIgnoreKey& key); // return > 0 if ignored
    static bool IgnoredIFP(const FIgnoreKey& key) NOEXCEPT;
    static void ClearIFP();

public: // singleton API
    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif
    using singleton_type::Destroy;

    static void Create() {
        singleton_type::Create();
    }

private:
    using FHitMap = MAP(Diagnostic, FIgnoreKey, FHitCount);
    TThreadSafe<FHitMap, EThreadBarrier::CriticalSection> _hits;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!USE_PPE_IGNORELIST
