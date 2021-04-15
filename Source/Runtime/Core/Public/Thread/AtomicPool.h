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
template <typename T, size_t _Chunks = 1, typename _Index = size_t, size_t _Align = CACHELINE_SIZE >
class TAtomicPool : Meta::FNonCopyableNorMovable {
    STATIC_ASSERT(_Chunks > 0);
    using mask_type = TAtomicBitMask<_Index>;

    struct CACHELINE_ALIGNED {
        mask_type Alloc{ UMax };
        mask_type Create{ 0 };
    }   _sets[_Chunks];

    using block_type = std::aligned_storage_t<sizeof(T), _Align>;
    block_type _storage[mask_type::Capacity * _Chunks];

public:
    using index_type = typename mask_type::value_type;
    STATIC_CONST_INTEGRAL(size_t, Capacity, mask_type::Capacity * _Chunks);

    TAtomicPool() = default;
#if USE_PPE_ASSERT
    ~TAtomicPool() {
        for (auto& set : _sets) {
            Assert_NoAssume(set.Alloc.AllTrue());
            Assert_NoAssume(set.Create.AllFalse()); // must call Clear_ReleaseMemory()
        }
    }
#endif

    NODISCARD bool Aliases(const T* p) const NOEXCEPT {
        return reinterpret_cast<const T*>(_storage) >= p
            && reinterpret_cast<const T*>(_storage + Capacity) > p;
    }

    template <typename _Ctor>
    NODISCARD T* Allocate(const _Ctor& ctor) NOEXCEPT {
        index_type alloc{ UMax };
        forrange(ch, 0, _Chunks) {
            auto& set = _sets[ch];
            if (set.Alloc.Assign(&alloc)) {
                T* const p = reinterpret_cast<T*>(_storage + ch * mask_type::Capacity + alloc);
                Assert_NoAssume(Aliases(p));

                if (set.Create.Set(alloc))
                    ctor(p);

                return p;
            }
        }

        return nullptr;
    }

    void Release(T* p) NOEXCEPT {
        Assert(p);
        Assert_NoAssume(Aliases(p));

        auto abs = checked_cast<index_type>(reinterpret_cast<block_type*>(p) - _storage);
        auto ch = abs / mask_type::Capacity;
        auto rel = abs - ch * mask_type::Capacity;
        Assert(ch < _Chunks);

        _sets[ch].Alloc.Release(rel);
    }

    template <typename _Dtor>
    void Clear_ReleaseMemory(const _Dtor& dtor) {
        forrange(ch, 0, _Chunks) {
            auto& set = _sets[ch];
            auto a = set.Alloc.Fetch();
            auto c = set.Create.Fetch();
            while (c) {
                index_type alloc = c.PopFront_AssumeNotEmpty();
                if (not a.Get(alloc))
                    dtor(reinterpret_cast<T*>(_storage + ch * mask_type::Capacity + alloc));
            }
        }

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(_storage, sizeof(_storage)));
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
