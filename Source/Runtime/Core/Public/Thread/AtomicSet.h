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
    STATIC_CONST_INTEGRAL(size_t, Capacity, mask_type::BitCount);

    explicit TAtomicBitMask(T init = UMax) NOEXCEPT
        : _data(init)
    {}

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
        return Fetch().Get(index);
    }

    bool Set(T index) NOEXCEPT {
        Assert(index < Capacity);
        const T mask{ 1u << index };
        return not (_data.fetch_or(mask, std::memory_order_relaxed) & mask);
    }

    bool Assign(T* pindex) NOEXCEPT {
        Assert(pindex);

        for (T bits{ _data.load(std::memory_order_relaxed) }; bits; ) {
            mask_type set{ bits };
            const T alloc = set.PopFront_AssumeNotEmpty();

            if (_data.compare_exchange_weak(bits, set.Data,
                    std::memory_order_release,
                    std::memory_order_relaxed) ) {
                Assert_NoAssume(alloc < Capacity);
                *pindex = alloc;
                return true;
            }
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
