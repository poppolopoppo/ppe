#pragma once

#include "Core.h"

#include "HAL/PlatformMemory.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/RefPtr.h"
#include "Meta/AlignedStorage.h"
#include "Meta/Functor.h"
#include "Thread/AtomicSet.h"

#if USE_PPE_MEMORYDOMAINS
#   include "Meta/TypeInfo.h"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Atomic pool of fixed size using a bit mask
//----------------------------------------------------------------------------
PRAGMA_MSVC_WARNING_PUSH()
PRAGMA_MSVC_WARNING_DISABLE(4324) // structure was padded due to alignment
template <typename T, size_t _Chunks = 1, typename _Index = size_t, size_t _Align = alignof(T) >
class TAtomicPool : Meta::FNonCopyableNorMovable {
    STATIC_ASSERT(_Chunks > 0);
    using mask_type = TAtomicBitMask<_Index>;

    struct CACHELINE_ALIGNED/* avoid false-sharing of those flags */ {
        mask_type Alloc{ mask_type::AllMask };
        mask_type Create{ mask_type::EmptyMask };
    }   _sets[_Chunks];

    using block_type = std::aligned_storage_t<sizeof(T), _Align>;
    block_type _storage[mask_type::Capacity * _Chunks];

#if USE_PPE_MEMORYDOMAINS
    mutable FAutoRegisterMemoryTracking _trackingData{
        Meta::type_info<TAtomicPool>.name,
        std::addressof(MEMORYDOMAIN_TRACKING_DATA(AtomicPool)) };
#endif

public:
    using index_type = typename mask_type::value_type;
    STATIC_CONST_INTEGRAL(size_t, Capacity, mask_type::Capacity * _Chunks);

#if USE_PPE_MEMORYDOMAINS
    TAtomicPool() NOEXCEPT {
        _trackingData.AllocateSystem(Capacity * sizeof(block_type));
    }
#else
    TAtomicPool() = default;
#endif
#if USE_PPE_ASSERT || USE_PPE_MEMORYDOMAINS
    ~TAtomicPool() {
        ONLY_IF_MEMORYDOMAINS(_trackingData.DeallocateSystem(Capacity * sizeof(block_type)) );
#   if USE_PPE_ASSERT
        for (auto& set : _sets) {
            Assert_NoAssume(set.Alloc.AllTrue());
            Assert_NoAssume(set.Create.AllFalse()); // must call Clear_ReleaseMemory()
        }
#   endif
    }
#endif

    _Index NumFreeBlocks() const {
        _Index cnt = 0;
        forrange(ch, 0, _Chunks)
            cnt += _sets[ch].Alloc.Fetch().Count();
        return cnt;
    }
    _Index NumCreatedBlocks() const {
        _Index cnt = 0;
        forrange(ch, 0, _Chunks)
            cnt += _sets[ch].Create.Fetch().Count();
        return cnt;
    }
    _Index NumCreatedAndFreeBlocks() const {
        _Index cnt = 0;
        forrange(ch, 0, _Chunks)
            cnt += (_sets[ch].Alloc.Fetch() & _sets[ch].Create.Fetch()).Count();
        return cnt;
    }

    NODISCARD bool Aliases(const T* p) const NOEXCEPT {
        return reinterpret_cast<const T*>(_storage) <= p
            && reinterpret_cast<const T*>(_storage + Capacity) > p;
    }

    NODISCARD index_type BlockIndex(const T* p) const NOEXCEPT {
        Assert_NoAssume(Aliases(p));
        return checked_cast<index_type>(p - reinterpret_cast<const T*>(_storage));
    }

    NODISCARD T* BlockAddress(index_type block) const NOEXCEPT {
        Assert_NoAssume(block < Capacity);
        return const_cast<T*>(reinterpret_cast<const T*>(_storage + block));
    }

    NODISCARD T* Allocate() NOEXCEPT {
        return Allocate(Meta::DefaultConstructor<T>);
    }
    template <typename _Ctor>
    NODISCARD T* Allocate(_Ctor&& ctor) NOEXCEPT {
        index_type alloc{ UMax };
        forrange(ch, 0, checked_cast<u32>(_Chunks)) {
            auto& set = _sets[ch];
            if (set.Alloc.Assign(&alloc)) {
                Assert_NoAssume(alloc < mask_type::Capacity);
                const index_type id = checked_cast<index_type>(ch * mask_type::Capacity + alloc);
                T* const p = reinterpret_cast<T*>(_storage + id);
                Assert_NoAssume(Aliases(p));

                ONLY_IF_MEMORYDOMAINS( _trackingData.AllocateUser(sizeof(block_type)) );

                if (set.Create.SetBit(alloc)) {
                    ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(p, sizeof(block_type)));
                    Meta::VariadicFunctor(ctor, p, id);
                }

                return p;
            }
        }

        return nullptr;
    }

    void ReleaseBlock(index_type block) NOEXCEPT {
        Release(BlockAddress(block));
    }
    void Release(const T* p) NOEXCEPT {
        Assert(p);
        Assert_NoAssume(Aliases(p));

        ONLY_IF_MEMORYDOMAINS( _trackingData.DeallocateUser(sizeof(block_type)) );

        auto abs = checked_cast<index_type>(reinterpret_cast<const block_type*>(p) - _storage);
        auto ch = abs / mask_type::Capacity;
        auto rel = abs - ch * mask_type::Capacity;
        Assert(ch < _Chunks);

        _sets[ch].Alloc.Release(rel);
    }

    void Clear_ReleaseMemory() {
        Clear_ReleaseMemory(Meta::Destructor<T>);
    }
    template <typename _Dtor>
    void Clear_ReleaseMemory(_Dtor&& dtor) {
        forrange(ch, 0, checked_cast<u32>(_Chunks)) {
            for (auto & set = _sets[ch];;) {
                // release only free blocks
                auto c = set.Create.Fetch();
                c &= set.Alloc.Fetch();
                if (not c)
                    break; // assume nothing to release

                // first acquire the block that is already created, but not allocated
                const index_type alloc = c.PopFront_AssumeNotEmpty();
                if (not set.Alloc.AcquireIFP(alloc))
                    continue; // retry if CAS failed

                // then release the block while allocated, if still created
                if (set.Create.UnsetBit(alloc)) {
                    const auto id = ch * mask_type::Capacity + alloc;
                    Meta::VariadicFunctor(dtor, reinterpret_cast<T*>(_storage + id), id);
                    ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(_storage + id, sizeof(block_type)));
                }

                // finally release the allocated block, but now it's destroyed
                Assert_NoAssume(not set.Create.Get(alloc));
                set.Alloc.Release(alloc);
            }
        }
        std::atomic_thread_fence(std::memory_order_release);
    }

};
PRAGMA_MSVC_WARNING_POP()
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
