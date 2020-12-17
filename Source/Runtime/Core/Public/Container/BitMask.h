#pragma once

#include "Core.h"

#include "HAL/PlatformMaths.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T = size_t>
struct TBitMask {
    STATIC_ASSERT(std::is_integral_v<T>);
    using word_t = T;

    static CONSTEXPR word_t One = word_t(1);
    static CONSTEXPR word_t AllMask = word_t(-1);
    static CONSTEXPR word_t BitCount = (sizeof(word_t) << 3);
    static CONSTEXPR word_t BitMask = (BitCount - 1);

    word_t Data;

    CONSTEXPR operator word_t () const NOEXCEPT { return Data; }

    u32 Count() const NOEXCEPT { return checked_cast<u32>(FPlatformMaths::popcnt(Data)); }

    template <u32 _Index>
    CONSTEXPR bool Get() const NOEXCEPT {
        STATIC_ASSERT(_Index < BitCount);
        return (!!((Data >> _Index) & 1));
    }

    CONSTEXPR bool Get(u32 index) const {
        Assert_NoAssume(index < BitCount);
        return (!!((Data >> index) & 1));
    }

    CONSTEXPR void Set(u32 index, bool value) { Assert(index < BitCount); Data = (value ? Data | (One << index) : Data & ~(One << index)); }

    CONSTEXPR void SetTrue(u32 index) { Assert(index < BitCount); Data |= One << index; }
    CONSTEXPR void SetFalse(u32 index) { Assert(index < BitCount); Data &= ~(One << index); }

    CONSTEXPR bool operator [](u32 index) const { return Get(index); }

    CONSTEXPR bool AllTrue() const NOEXCEPT { return ((Data & AllMask) == AllMask); }
    CONSTEXPR bool AllFalse() const NOEXCEPT { return ((Data & AllMask) == 0); }

    CONSTEXPR bool AnyTrue() const NOEXCEPT { return (Data != 0); }
    CONSTEXPR bool AnyFalse() const NOEXCEPT { return (Data != AllMask); }

    CONSTEXPR void ResetAll(bool value) NOEXCEPT { Data = (value ? AllMask : 0); }

    CONSTEXPR TBitMask Invert() const NOEXCEPT { return TBitMask{ ~Data }; }

    CONSTEXPR TBitMask operator &(TBitMask other) const NOEXCEPT { return TBitMask{ Data & other.Data }; }
    CONSTEXPR TBitMask operator |(TBitMask other) const NOEXCEPT { return TBitMask{ Data | other.Data }; }
    CONSTEXPR TBitMask operator ^(TBitMask other) const NOEXCEPT { return TBitMask{ Data ^ other.Data }; }

    CONSTEXPR TBitMask operator <<(word_t lshift) const NOEXCEPT { return TBitMask{ Data << lshift }; }
    CONSTEXPR TBitMask operator >>(word_t rshift) const NOEXCEPT { return TBitMask{ Data >> rshift }; }

    word_t CountTrailingZeros() const NOEXCEPT {
        return FPlatformMaths::ctz(Data);
    }

    word_t FirstBitSet_AssumeNotEmpty() const NOEXCEPT {
        Assert(Data);
        return FPlatformMaths::tzcnt(Data);
    }
    word_t FirstBitSet_Unsafe() const NOEXCEPT {
        return FPlatformMaths::tzcnt(Data);
    }

    word_t LastBitSet_AssumeNotEmpty() const NOEXCEPT {
        Assert(Data);
        return FPlatformMaths::lzcnt(Data);
    }

    word_t PopFront() NOEXCEPT { // return 0 if empty or bit index + 1
        const word_t front = (Data ? FPlatformMaths::tzcnt(Data) : word_t(-1));
        Data &= ~(One << front); // when empty : 1 << INDEX_NONE = 1 << 0xFFFFFFFF = 0
        return (front + 1);
    }

    word_t PopFront_AssumeNotEmpty() NOEXCEPT {
        Assert(Data);
        const word_t front = FPlatformMaths::tzcnt(Data);
        Data &= ~(One << front);
        return front;
    }

    word_t PopFront_Unsafe() NOEXCEPT {
        const word_t front = FPlatformMaths::tzcnt(Data);
        Data &= ~(One << front);
        return front;
    }

    static CONSTEXPR TBitMask SetFirstN(word_t n) NOEXCEPT { return TBitMask{ AllMask >> (BitCount - n) }; }
    static CONSTEXPR TBitMask UnsetFirstN(word_t n) NOEXCEPT { return TBitMask{ AllMask << n }; }
    static CONSTEXPR TBitMask SetLastN(word_t n)  NOEXCEPT { return TBitMask{ ~SetFirstN(BitCount - n) }; }

    word_t Allocate(size_t n) NOEXCEPT { // return 0 if failed or bit index + 1
        Assert(n > 1);
        Assert(n <= BitCount);

        const word_t mask = (One << word_t(n)) - 1;

        for (word_t m = Data; m;) {
            const word_t front = FPlatformMaths::tzcnt(m);
            const word_t alloc = (mask << front);
            if ((alloc & m) == alloc) {
                Data &= ~alloc;
                return (front + 1);
            }
            m &= ~(One << front);
        }

        return 0;
    }

    word_t Reallocate(size_t i, size_t o, size_t n) NOEXCEPT { // return 0 if failed or bit index + 1
        Assert(o);
        Assert(n);
        Assert(n <= BitCount);
        Assert(i + o <= BitCount);

        const word_t omask = ((One << word_t(o)) - 1) << word_t(i);
        Assert_NoAssume(not (omask & Data));

        // temporary remove the current allocation
        Data |= omask;

        // then try to allocate normally the bigger block
        if (const word_t j = Allocate(n)) {
            return j;
        }
        // restore original allocation if reallocate failed
        else {
            Data &= ~omask;
            return 0;
        }
    }

    void Deallocate(size_t i, size_t n) NOEXCEPT {
        Assert(i + n <= BitCount);

        const word_t mask = ((One << word_t(n)) - 1) << word_t(i);
        Assert_NoAssume(not (mask & Data));

        Data |= mask;
    }
};
//----------------------------------------------------------------------------
template <>
struct TBitMask<u128> {
    using word_t = u128;

    static CONSTEXPR word_t One = u128{ 1, 0 };
    static CONSTEXPR word_t Zero = u128{ 0, 0 };
    static CONSTEXPR word_t AllMask = u128{ u64(-1), u64(-1) };
    static CONSTEXPR size_t BitCount = (sizeof(u128) << 3);

    word_t Data;

    CONSTEXPR operator word_t () const NOEXCEPT { return Data; }

    u32 Count() const NOEXCEPT { return checked_cast<u32>(FPlatformMaths::popcnt(Data)); }

    template <u32 _Index>
    CONSTEXPR bool Get() const NOEXCEPT {
        STATIC_ASSERT(_Index < BitCount);
        return !!(((_Index < 64 ? Data.lo : Data.hi) >> (_Index & 63)) & 1);
    }

    CONSTEXPR bool Get(u32 index) const {
        Assert_NoAssume(index < BitCount);
        return !!(((index < 64 ? Data.lo : Data.hi) >> (index & 63)) & 1);
    }

    CONSTEXPR void Set(u32 index, bool value) {
        Assert(index < BitCount);
        u64& word = (index < 64 ? Data.lo : Data.hi);
        index &= 63;
        word = (value ? word | (u64(1) << index) : word & ~(u64(1) << index));
    }

    CONSTEXPR void SetTrue(u32 index) { Assert(index < BitCount); (index < 64 ? Data.lo : Data.hi) |= (u64(1) << (index & 63)); }
    CONSTEXPR void SetFalse(u32 index) { Assert(index < BitCount); (index < 64 ? Data.lo : Data.hi) &= ~(u64(1) << (index & 63)); }

    CONSTEXPR bool operator [](u32 index) const { return Get(index); }

    CONSTEXPR bool AllTrue() const NOEXCEPT { return (Data == AllMask); }
    CONSTEXPR bool AllFalse() const NOEXCEPT { return (Data == Zero); }

    CONSTEXPR bool AnyTrue() const NOEXCEPT { return (Data != Zero); }
    CONSTEXPR bool AnyFalse() const NOEXCEPT { return (Data != AllMask); }

    CONSTEXPR void ResetAll(bool value) NOEXCEPT { Data = (value ? AllMask : Zero); }

    CONSTEXPR TBitMask Invert() const NOEXCEPT { return TBitMask{ { ~Data.lo, ~Data.hi } }; }

    u64 CountTrailingZeros() const NOEXCEPT {
        return FPlatformMaths::ctz(Data);
    }

    u64 FirstBitSet_AssumeNotEmpty() const NOEXCEPT {
        Assert(Data.lo | Data.hi);
        return FPlatformMaths::tzcnt(Data);
    }

    u64 FirstBitSet_Unsafe() const NOEXCEPT {
        return FPlatformMaths::tzcnt(Data);
    }

    u64 LastBitSet_AssumeNotEmpty() const NOEXCEPT {
        Assert(Data.lo | Data.hi);
        return FPlatformMaths::lzcnt(Data);
    }
};
//----------------------------------------------------------------------------
using FBitMask = TBitMask<>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
