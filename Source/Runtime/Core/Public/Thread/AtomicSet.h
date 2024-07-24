#pragma once

#include "Core.h"

#include "Thread/AtomicSpinLock.h" // NotifyAllAtomicBarrier/SpinAtomicBarrier()
#include "Container/BitMask.h"

#include <atomic>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// BitSet using atomic CAS
//----------------------------------------------------------------------------
template <typename T>
class TAtomicBitMask {
public:
    using value_type = T;
    using mask_type = TBitMask<T>;
    STATIC_CONST_INTEGRAL(u32, Capacity, mask_type::BitCount);

    STATIC_CONST_INTEGRAL(mask_type, AllMask, mask_type{ mask_type::AllMask });
    STATIC_CONST_INTEGRAL(mask_type, EmptyMask, mask_type{ 0 });

    TAtomicBitMask() = default;

    explicit TAtomicBitMask(mask_type init = AllMask) NOEXCEPT
        : _data(init)
    {}

    std::atomic<T>& Data() { return _data; }

    NODISCARD mask_type Fetch() const NOEXCEPT {
        return { _data.load(std::memory_order_relaxed) };
    }
    NODISCARD bool AllTrue() NOEXCEPT {
        return Fetch().AllTrue();
    }
    NODISCARD bool AllFalse() NOEXCEPT {
        return Fetch().AllFalse();
    }
    NODISCARD bool Get(T index) NOEXCEPT {
        Assert(index < Capacity);
        return Fetch().Get(static_cast<u32>(index));
    }

    bool SetBit(T index) NOEXCEPT {
        Assert(index < Capacity);
        const T mask{ T(1u) << index };
        return not (_data.fetch_or(mask, std::memory_order_relaxed) & mask);
    }
    bool UnsetBit(T index) NOEXCEPT {
        Assert(index < Capacity);
        const T mask{ T(1u) << index };
        return !!(_data.fetch_and(~mask, std::memory_order_relaxed) & mask);
    }

    void SetAllTrue(std::memory_order order = std::memory_order_relaxed) NOEXCEPT { _data.store(AllMask, order); }
    void SetAllFalse(std::memory_order order = std::memory_order_relaxed) NOEXCEPT { _data.store(0, order); }

    void ResetMask(mask_type mask) {
        _data.store(mask.Data);
    }

    NODISCARD bool Cas(T& expected, T exchange) NOEXCEPT {
        return _data.compare_exchange_weak(expected, exchange,
            std::memory_order_release,
            std::memory_order_relaxed);
    }

    NODISCARD bool Assign(T* pindex) NOEXCEPT {
        Assert(pindex);

        for (T bits{ _data.load(std::memory_order_relaxed) }; bits; ) {
            mask_type set{ bits };
            const T alloc = set.PopFront_AssumeNotEmpty();

            if (Cas(bits, set.Data)) {
                Assert_NoAssume(alloc < Capacity);
                *pindex = alloc;
                return true;
            }
        }

        return false;
    }

    NODISCARD bool AcquireIFP(T index) NOEXCEPT {
        mask_type org = Fetch();
        if (org.Get(checked_cast<u32>(index))) {
            mask_type exh = org;
            exh.SetFalse(static_cast<u32>(index));
            return Cas(org.Data, exh.Data);
        }
        return false;
    }

    void Release(T index) NOEXCEPT {
        Verify(SetBit(index));
    }

    // must use NotifyDeallocate() instead of Release()
    NODISCARD T WaitAllocate() NOEXCEPT {
        i32 backoff = 0;
        for (T bits{ _data.load(std::memory_order_relaxed) };; ) {
            if (Likely(bits)) {
                mask_type set{ bits };
                const T alloc = set.PopFront_AssumeNotEmpty();

                if (Cas(bits, set.Data)) {
                    Assert_NoAssume(alloc < Capacity);
                    return alloc;
                }
            }
            else {
                details::SpinAtomicBarrier(&_data, bits, backoff);
                bits = _data.load(std::memory_order_relaxed);
            }
        }
    }
    NODISCARD T WaitAllocateRoll(u8 seed) NOEXCEPT {
        i32 backoff = 0;
        for (T bits{ _data.load(std::memory_order_relaxed) };; ) {
            if (Likely(bits)) {
                mask_type set{ bits };

                // rotate bitset to get uniform distribution, instead of always stressing the first entries
                set.RotateRight(seed);
                const T alloc = ((set.PopFront_AssumeNotEmpty() + seed) % mask_type::BitCount);
                set.RotateLeft(seed);

                if (Cas(bits, set.Data)) {
                    Assert_NoAssume(alloc < Capacity);
                    return alloc;
                }
            }
            else {
                details::SpinAtomicBarrier(&_data, bits, backoff);
                bits = _data.load(std::memory_order_relaxed);
            }
        }
    }
    void WaitAcquire(T index) NOEXCEPT {
        Assert_NoAssume(index < Capacity);
        i32 backoff = 0;
        for (T bits{ _data.load(std::memory_order_relaxed) };; ) {
            mask_type set{ bits };
            if (Likely(set.Get(index))) {
                set.SetFalse(index);

                if (Cas(bits, set.Data)) {
                    return;
                }
            }
            else {
                details::SpinAtomicBarrier(&_data, bits, backoff);
                bits = _data.load(std::memory_order_relaxed);
            }
        }
    }
    void NotifyDeallocate(T index) {
        Release(index);
        details::NotifyOneAtomicBarrier(&_data);
    }

private:
    STATIC_ASSERT(std::atomic<T>::is_always_lock_free);
    std::atomic<T> _data;
};
//----------------------------------------------------------------------------
using FAtomicBitMask = TAtomicBitMask<size_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
