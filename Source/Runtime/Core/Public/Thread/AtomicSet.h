#pragma once

#include "Core.h"

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
    STATIC_CONST_INTEGRAL(value_type, AllMask, mask_type::AllMask);

    TAtomicBitMask() = default;
    explicit TAtomicBitMask(T init = UMax) NOEXCEPT
        : _data(init)
    {}

    std::atomic<T>& Data() { return _data; }

    mask_type Fetch() const NOEXCEPT {
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

    bool Set(T index) NOEXCEPT {
        Assert(index < Capacity);
        const T mask{ T(1u) << index };
        return not (_data.fetch_or(mask, std::memory_order_relaxed) & mask);
    }
    bool Unset(T index) NOEXCEPT {
        Assert(index < Capacity);
        const T mask{ T(1u) << index };
        return !!(_data.fetch_and(~mask, std::memory_order_relaxed) & mask);
    }

    void SetAllTrue(std::memory_order order = std::memory_order_relaxed) NOEXCEPT { _data.store(AllMask, order); }
    void SetAllFalse(std::memory_order order = std::memory_order_relaxed) NOEXCEPT { _data.store(0, order); }

    bool Cas(T& expected, T exchange) NOEXCEPT {
        return _data.compare_exchange_weak(expected, exchange,
            std::memory_order_release,
            std::memory_order_relaxed);
    }

    bool Assign(T* pindex) NOEXCEPT {
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

    bool AcquireIFP(T index) NOEXCEPT {
        mask_type org = Fetch();
        if (org.Get(checked_cast<u32>(index))) {
            mask_type exh = org;
            exh.SetFalse(static_cast<u32>(index));
            return Cas(org.Data, exh.Data);
        }
        return false;
    }

    void Release(T index) NOEXCEPT {
        Verify(Set(index));
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
