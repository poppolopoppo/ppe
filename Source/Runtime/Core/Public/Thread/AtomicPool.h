#pragma once

#include "Core.h"
#include "HAL/PlatformMemory.h"
#include "Meta/AlignedStorage.h"

#include "Thread/AtomicSet.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Atomic pool of fixed size using a bit mask
//----------------------------------------------------------------------------
template <typename T, typename _Index = size_t, size_t _Align = CACHELINE_SIZE >
class TAtomicPool : Meta::FNonCopyableNorMovable {
    using mask_type = TAtomicBitMask<_Index>;

    struct CACHELINE_ALIGNED {
        mask_type Alloc{ UMax };
        mask_type Create{ 0 };
    }   _set;

    using block_type = std::aligned_storage_t<sizeof(T), _Align>;
    block_type _storage[mask_type::Capacity];

public:
    using index_type = typename mask_type::value_type;
    STATIC_CONST_INTEGRAL(size_t, Capacity, mask_type::Capacity);

    TAtomicPool() = default;
#if USE_PPE_ASSERT
    ~TAtomicPool() {
        Assert_NoAssume(_set.Alloc.AllTrue());
        Assert_NoAssume(_set.Create.AllFalse()); // must call Clear_ReleaseMemory()
    }
#endif

    NODISCARD bool Aliases(const T* p) const NOEXCEPT {
        return reinterpret_cast<const T*>(_storage) >= p
            && reinterpret_cast<const T*>(_storage + Capacity) > p;
    }

    template <typename _Ctor>
    NODISCARD T* Allocate(const _Ctor& ctor) NOEXCEPT {
        index_type alloc{ UMax };
        if (_set.Alloc.Assign(&alloc)) {
            T* const p = reinterpret_cast<T*>(_storage + alloc);
            Assert_NoAssume(Aliases(p));

            if (_set.Create.Set(alloc))
                ctor(p);

            return p;
        }

        return nullptr;
    }

    void Release(T* p) NOEXCEPT {
        Assert(p);
        Assert_NoAssume(Aliases(p));

        auto index = checked_cast<index_type>(reinterpret_cast<block_type*>(p) - _storage);
        _set.Alloc.Release(index);
    }

    template <typename _Dtor>
    void Clear_ReleaseMemory(const _Dtor& dtor) {
        auto a = _set.Alloc.Fetch();
        auto c = _set.Create.Fetch();
        while (c) {
            index_type alloc = c.PopFront_AssumeNotEmpty();
            if (not a.Get(alloc))
                dtor(reinterpret_cast<T*>(_storage + alloc));
        }

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(_storage, sizeof(_storage)));
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
