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

    static constexpr word_t GOne = word_t(1);
    static constexpr word_t GAllMask = word_t(-1);
    static constexpr word_t GBitCount = (sizeof(word_t)<<3);
    static constexpr word_t GBitMask = (GBitCount - 1);

    word_t Data;

    static CONSTEXPR TBitMask SetFirstN(word_t n) NOEXCEPT { return TBitMask{ GAllMask >> (GBitCount - n) }; }
    static CONSTEXPR TBitMask SetLastN(word_t n)  NOEXCEPT { return TBitMask{ ~SetFirstN(GBitCount - n)   }; }

    CONSTEXPR operator word_t () const NOEXCEPT { return Data; }

    size_t Count() const NOEXCEPT { return FPlatformMaths::popcnt(Data); }

    template <size_t _Index>
    CONSTEXPR bool Get() const NOEXCEPT {
        STATIC_ASSERT(_Index < GBitCount);
        return (0 != (Data & (word_t(1) << _Index)));
    }

    bool Get(size_t index) const { Assert(index < GBitCount); return ((Data & (GOne<<index)) != 0); }
    void Set(size_t index, bool value) { Assert(index < GBitCount); Data = (value ? Data|(GOne<<index) : Data&~(GOne<<index)); }

    void SetTrue(size_t index) { Assert(index < GBitCount); Data |= GOne<<index; }
    void SetFalse(size_t index) { Assert(index < GBitCount); Data &= ~(GOne<<index); }

    CONSTEXPR bool operator [](size_t index) const { return Get(index); }

    CONSTEXPR bool AllTrue() const NOEXCEPT { return ((Data & GAllMask) == GAllMask); }
    CONSTEXPR bool AllFalse() const NOEXCEPT { return ((Data & GAllMask) == 0); }

    CONSTEXPR bool AnyTrue() const NOEXCEPT { return (Data != 0); }
    CONSTEXPR bool AnyFalse() const NOEXCEPT { return (Data != GAllMask); }

    CONSTEXPR void ResetAll(bool value) NOEXCEPT { Data = (value ? GAllMask : 0); }

    CONSTEXPR TBitMask Invert() const NOEXCEPT { return TBitMask{ ~Data }; }

    CONSTEXPR TBitMask operator &(TBitMask other) const NOEXCEPT { return TBitMask{ Data & other.Data }; }
    CONSTEXPR TBitMask operator |(TBitMask other) const NOEXCEPT { return TBitMask{ Data | other.Data }; }
    CONSTEXPR TBitMask operator ^(TBitMask other) const NOEXCEPT { return TBitMask{ Data ^ other.Data }; }

    CONSTEXPR TBitMask operator <<(word_t lshift) const NOEXCEPT { return TBitMask{ Data << lshift }; }
    CONSTEXPR TBitMask operator >>(word_t rshift) const NOEXCEPT { return TBitMask{ Data >> rshift }; }

    word_t FirstBitSet_AssumeNotEmpty() const NOEXCEPT { return FPlatformMaths::tzcnt(Data); }
    word_t LastBitSet_AssumeNotEmpty() const NOEXCEPT { return FPlatformMaths::lzcnt(Data); }

    word_t PopFront() NOEXCEPT { // return 0 if empty of (LSB index + 1)
        const word_t front = (Data ? FPlatformMaths::tzcnt(Data) : INDEX_NONE);
        Data &= ~(GOne<<front); // when empty : 1 << INDEX_NONE = 1 << 0xFFFFFFFF = 0
        return (front + 1);
    }

    word_t PopFront_AssumeNotEmpty() NOEXCEPT {
        const word_t front = FPlatformMaths::tzcnt(Data);
        Data &= ~(GOne<<front); // when empty : 1 << INDEX_NONE = 1 << 0xFFFFFFFF = 0
        return front;
    }

    word_t PopBack() NOEXCEPT { // return 0 if empty of (MSB index + 1)
        const word_t back = (Data ? FPlatformMaths::lzcnt(Data) : INDEX_NONE);
        Data &= ~(GOne<<back); // when empty : 1 << INDEX_NONE = 1 << 0xFFFFFFFF = 0
        return (back + 1);
    }
};
//----------------------------------------------------------------------------
using FBitMask = TBitMask<>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
