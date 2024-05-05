#pragma once

#include "Core.h"

#include "HAL/PlatformMaths.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T = size_t, int SignifiantBits = sizeof(T) * 8>
struct TBitMask {
    STATIC_ASSERT(std::is_integral_v<T>);
    using word_t = T;

    static CONSTEXPR u32 BitCount = (sizeof(word_t) << 3);
    static CONSTEXPR word_t ExtraBits = (sizeof(T) * 8 - SignifiantBits);
    static CONSTEXPR word_t One = word_t(1);
    static CONSTEXPR word_t AllMask = (word_t(-1) >> ExtraBits);

    word_t Data;

    CONSTEXPR operator word_t () const NOEXCEPT { return Data; }

    NODISCARD CONSTEXPR bool operator [](u32 index) const { return Get(index); }

    template <u32 _Index>
    NODISCARD CONSTEXPR bool Get() const NOEXCEPT {
        STATIC_ASSERT(_Index < BitCount);
        return (!!((Data >> _Index) & 1));
    }

    NODISCARD CONSTEXPR bool Get(u32 index) const {
        Assert_NoAssume(index < BitCount);
        return (!!((Data >> index) & 1));
    }

    CONSTEXPR void Set(u32 index, bool value) { Assert(index < BitCount); Data = (value ? Data | (One << index) : Data & ~(One << index)); }

    CONSTEXPR void SetTrue(u32 index) { Assert(index < BitCount); Data |= One << index; }
    CONSTEXPR void SetFalse(u32 index) { Assert(index < BitCount); Data &= ~(One << index); }

    NODISCARD CONSTEXPR bool AllTrue() const NOEXCEPT { return ((Data & AllMask) == AllMask); }
    NODISCARD CONSTEXPR bool AllFalse() const NOEXCEPT { return ((Data & AllMask) == 0); }

    NODISCARD CONSTEXPR bool AnyTrue() const NOEXCEPT { return (Data != 0); }
    NODISCARD CONSTEXPR bool AnyFalse() const NOEXCEPT { return (Data != AllMask); }

    CONSTEXPR void SetAllTrue() NOEXCEPT { Data = AllMask; }
    CONSTEXPR void SetAllFalse() NOEXCEPT { Data = 0; }
    CONSTEXPR void ResetAll(bool value) NOEXCEPT { Data = (value ? AllMask : 0); }

    NODISCARD CONSTEXPR TBitMask Invert() const NOEXCEPT { return TBitMask{ ~Data & AllMask }; }

    NODISCARD CONSTEXPR bool Contains(TBitMask other) const NOEXCEPT { return (Data & other.Data) == other.Data; }
    NODISCARD CONSTEXPR bool Intersects(TBitMask other) const NOEXCEPT { return !!(Data & other.Data); }

    CONSTEXPR TBitMask& operator &=(TBitMask other) NOEXCEPT { Data &= other.Data; return (*this); }
    CONSTEXPR TBitMask& operator |=(TBitMask other) NOEXCEPT { Data |= other.Data; return (*this); }
    CONSTEXPR TBitMask& operator ^=(TBitMask other) NOEXCEPT { Data ^= other.Data; return (*this); }
    CONSTEXPR TBitMask& operator -=(TBitMask other) NOEXCEPT { Data &= ~other.Data; return (*this); }

    CONSTEXPR TBitMask operator &(TBitMask other) const NOEXCEPT { return TBitMask{ Data & other.Data }; }
    CONSTEXPR TBitMask operator |(TBitMask other) const NOEXCEPT { return TBitMask{ Data | other.Data }; }
    CONSTEXPR TBitMask operator ^(TBitMask other) const NOEXCEPT { return TBitMask{ Data ^ other.Data }; }
    CONSTEXPR TBitMask operator -(TBitMask other) const NOEXCEPT { return TBitMask{ Data & ~other.Data }; }

    CONSTEXPR TBitMask operator <<(word_t lshift) const NOEXCEPT { return TBitMask{ Data << lshift }; }
    CONSTEXPR TBitMask operator >>(word_t rshift) const NOEXCEPT { return TBitMask{ Data >> rshift }; }

    NODISCARD u32 Count() const NOEXCEPT {
        return checked_cast<u32>(FPlatformMaths::popcnt(Data));
    }
    NODISCARD word_t LowestBitSet() const NOEXCEPT {
        return FPlatformMaths::ctz(Data);
    }
    NODISCARD word_t CountTrailingZeros() const NOEXCEPT {
        return FPlatformMaths::ctz(Data);
    }
    NODISCARD word_t CountLeadingZeros() const NOEXCEPT {
        return FPlatformMaths::lzcnt(Data << ExtraBits);
    }

    NODISCARD word_t FirstBitSet_AssumeNotEmpty() const NOEXCEPT {
        Assert(Data);
        return FPlatformMaths::tzcnt(Data);
    }
    NODISCARD word_t FirstBitSet_Unsafe() const NOEXCEPT {
        return FPlatformMaths::tzcnt(Data);
    }

    NODISCARD word_t LastBitSet_AssumeNotEmpty() const NOEXCEPT {
        Assert(Data);
        return FPlatformMaths::lzcnt(Data << ExtraBits);
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

    void RotateLeft(u8 n) NOEXCEPT {
        Data = FPlatformMaths::rotl(Data, n);
    }
    void RotateRight(u8 n) NOEXCEPT {
        Data = FPlatformMaths::rotr(Data, n);
    }

    NODISCARD CONSTEXPR u32 operator *() const { return LowestBitSet(); }
    CONSTEXPR TBitMask& operator ++() {
        Data &= (Data - 1);
        return (*this);
    }

    CONSTEXPR TBitMask begin() const { return (*this); }
    CONSTEXPR TBitMask end() const { return TBitMask{ 0 }; }

    NODISCARD friend CONSTEXPR bool operator ==(TBitMask lhs, TBitMask rhs) { return (lhs.Data == rhs.Data); }
    NODISCARD friend CONSTEXPR bool operator !=(TBitMask lhs, TBitMask rhs) { return (lhs.Data != rhs.Data); }

    NODISCARD static CONSTEXPR TBitMask SetFirstN(word_t n) NOEXCEPT { return TBitMask{ AllMask >> (BitCount - n) }; }
    NODISCARD static CONSTEXPR TBitMask UnsetFirstN(word_t n) NOEXCEPT { return TBitMask{ AllMask << n }; }
    NODISCARD static CONSTEXPR TBitMask SetLastN(word_t n)  NOEXCEPT { return TBitMask{ ~SetFirstN(BitCount - n) }; }

    NODISCARD word_t Allocate(size_t n) NOEXCEPT { // return 0 if failed or bit index + 1
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

    NODISCARD word_t Reallocate(size_t i, size_t o, size_t n) NOEXCEPT { // return 0 if failed or bit index + 1
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
    static CONSTEXPR u32 BitCount = (sizeof(u128) << 3);

    word_t Data;

    CONSTEXPR operator word_t () const NOEXCEPT { return Data; }

    CONSTEXPR bool operator [](u32 index) const { return Get(index); }

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

    CONSTEXPR bool AllTrue() const NOEXCEPT { return (Data == AllMask); }
    CONSTEXPR bool AllFalse() const NOEXCEPT { return (Data == Zero); }

    CONSTEXPR bool AnyTrue() const NOEXCEPT { return (Data != Zero); }
    CONSTEXPR bool AnyFalse() const NOEXCEPT { return (Data != AllMask); }

    CONSTEXPR void SetAllTrue() NOEXCEPT { Data = AllMask; }
    CONSTEXPR void SetAllFalse() NOEXCEPT { Data = Zero; }
    CONSTEXPR void ResetAll(bool value) NOEXCEPT { Data = (value ? AllMask : Zero); }

    CONSTEXPR TBitMask Invert() const NOEXCEPT { return TBitMask{ { ~Data.lo, ~Data.hi } }; }

    u32 Count() const NOEXCEPT { return checked_cast<u32>(FPlatformMaths::popcnt(Data)); }

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
PPE_ASSUME_TEMPLATE_AS_POD(COMMA_PROTECT(TBitMask<T, SignifiantBits>), typename T, int SignifiantBits)
//----------------------------------------------------------------------------
template <typename _Integral, class = Meta::TEnableIf<std::is_integral_v<_Integral>> >
CONSTEXPR CONSTF auto MakeBitMask(_Integral flags) {
    using integral_t = std::conditional_t < // clamp small enums to u32 for lzcnt/tzcnt overloads
        sizeof(_Integral) < sizeof(u32), u32, std::make_unsigned_t<_Integral> > ;
    return TBitMask<integral_t, Meta::BitCount<_Integral>>{ checked_cast<integral_t>(flags) };
}
//----------------------------------------------------------------------------
template <typename _Enum, class = Meta::TEnableIf<Meta::enum_is_flags_v<_Enum>> >
CONSTEXPR CONSTF auto MakeEnumBitMask(_Enum flags) {
    return MakeBitMask( Meta::EnumOrd(flags) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t N, typename T = size_t>
struct TFixedSizeBitMask {
    using word_t = T;
    using bitmask_t = TBitMask<word_t>;
    STATIC_CONST_INTEGRAL(u32, Capacity, N);
    STATIC_CONST_INTEGRAL(u32, BitsPerWord, bitmask_t::BitCount);
    STATIC_CONST_INTEGRAL(u32, NumWords, (Capacity + BitsPerWord - 1) / BitsPerWord);
    STATIC_CONST_INTEGRAL(word_t, RemainerMask, Capacity % BitsPerWord ? ~((word_t(1) << (Capacity % BitsPerWord)) - 1) : 0);

    bitmask_t Words[NumWords]{};

    TFixedSizeBitMask() = default;

    TFixedSizeBitMask(const TFixedSizeBitMask& ) = default;
    TFixedSizeBitMask& operator =(const TFixedSizeBitMask& ) = default;

    CONSTEXPR TFixedSizeBitMask(Meta::FForceInit) : Words{} {}
    CONSTEXPR TFixedSizeBitMask(std::initializer_list<u32> trueBits) : Words{} {
        SetTrue(trueBits);
    }
    template <typename _It>
    CONSTEXPR TFixedSizeBitMask(_It first, _It last) : Words{} {
        for (; first != last; ++first)
            SetTrue(*first);
    }

    CONSTEXPR bool operator [](u32 index) const { return Get(index); }

    CONSTEXPR bool Get(u32 index) const {
        Assert_NoAssume(index < N);
        return Words[index / BitsPerWord].Get(index % BitsPerWord);
    }
    CONSTEXPR void Set(u32 index, bool value) {
        Assert_NoAssume(index < N);
        Words[index / BitsPerWord].Set(index % BitsPerWord, value);
    }
    CONSTEXPR void SetTrue(u32 index) {
        Assert_NoAssume(index < N);
        Words[index / BitsPerWord].SetTrue(index % BitsPerWord);
    }
    CONSTEXPR void SetFalse(u32 index) {
        Assert_NoAssume(index < N);
        Words[index / BitsPerWord].SetFalse(index % BitsPerWord);
    }

    CONSTEXPR void Set(std::initializer_list<u32> bits, bool value) {
        if (value)
            SetTrue(bits);
        else
            SetFalse(bits);
    }
    CONSTEXPR void Append(std::initializer_list<u32> bits) { SetTrue(bits); }
    CONSTEXPR void SetTrue(std::initializer_list<u32> bits) {
        for (auto bit : bits)
            SetTrue(bit);
    }
    CONSTEXPR void SetFalse(std::initializer_list<u32> bits) {
        for (auto bit : bits)
            SetFalse(bit);
    }

    CONSTEXPR TFixedSizeBitMask& operator <<(u32 bit) {
        SetTrue(bit);
        return (*this);
    }
    CONSTEXPR TFixedSizeBitMask& operator <<(std::initializer_list<u32> bits) {
        SetTrue(bits);
        return (*this);
    }

    // enum interface, only valid when not using ENUM_FLAGS():
    template <typename _Enum, Meta::TEnableIf<Meta::enum_no_flags_v<_Enum>>* = nullptr >
    CONSTEXPR bool Get(_Enum index) const { return Get(Meta::EnumOrd(index)); }
    template <typename _Enum, Meta::TEnableIf<Meta::enum_no_flags_v<_Enum>>* = nullptr >
    CONSTEXPR void Set(_Enum index, bool value) { Set(Meta::EnumOrd(index), value); }
    template <typename _Enum, Meta::TEnableIf<Meta::enum_no_flags_v<_Enum>>* = nullptr >
    CONSTEXPR void SetTrue(_Enum index) { SetTrue(Meta::EnumOrd(index)); }
    template <typename _Enum, Meta::TEnableIf<Meta::enum_no_flags_v<_Enum>>* = nullptr >
    CONSTEXPR void SetFalse(_Enum index) { SetFalse(Meta::EnumOrd(index)); }
    template <typename _Enum, Meta::TEnableIf<Meta::enum_no_flags_v<_Enum>>* = nullptr >
    CONSTEXPR void Append(std::initializer_list<_Enum> indices) { SetTrue(indices); }
    template <typename _Enum, Meta::TEnableIf<Meta::enum_no_flags_v<_Enum>>* = nullptr >
    CONSTEXPR void Set(std::initializer_list<_Enum> indices, bool value) { value ? SetTrue(indices) : SetFalse(indices); }
    template <typename _Enum, Meta::TEnableIf<Meta::enum_no_flags_v<_Enum>>* = nullptr >
    CONSTEXPR void SetTrue(std::initializer_list<_Enum> indices) { for (auto x : indices) SetTrue(x); }
    template <typename _Enum, Meta::TEnableIf<Meta::enum_no_flags_v<_Enum>>* = nullptr >
    CONSTEXPR void SetFalse(std::initializer_list<_Enum> indices) { for (auto x : indices) SetFalse(x); }

    CONSTEXPR bool AllTrue() const NOEXCEPT {
        forrange(i, 0, NumWords - 1)
            if (not Words[i].AllTrue())
                return false;
        return (Words[NumWords - 1].Data | RemainerMask) == bitmask_t::AllMask;
    }
    CONSTEXPR bool AllFalse() const NOEXCEPT {
        for (const bitmask_t& bm : Words)
            if (not bm.AllFalse())
                return false;
        return true;
    }

    CONSTEXPR bool AnyTrue() const NOEXCEPT {
        for (const bitmask_t& bm : Words)
            if (bm.AnyTrue())
                return true;
        return false;
    }
    CONSTEXPR bool AnyFalse() const NOEXCEPT {
        forrange(i, 0, NumWords - 1)
            if (Words[i].AnyFalse())
                return true;
        return (Words[NumWords - 1].Data | RemainerMask) != bitmask_t::AllMask;
    }

    CONSTEXPR void SetAllTrue() NOEXCEPT {
        forrange(i, 0, NumWords - 1)
            Words[i].SetAllTrue();
        Words[NumWords - 1].Data = ~RemainerMask;
    }
    CONSTEXPR void SetAllFalse() NOEXCEPT {
        for (bitmask_t& bm : Words)
            bm.SetAllFalse();
    }
    CONSTEXPR void ResetAll(bool value) NOEXCEPT {
        if (value)
            SetAllTrue();
        else
            SetAllFalse();
    }

    CONSTEXPR TFixedSizeBitMask Invert() const NOEXCEPT {
        TFixedSizeBitMask result;
        forrange(i, 0, NumWords - 1)
            result.Words[i] = Words[i].Invert();
        result.Words[NumWords - 1].Data = (Words[NumWords - 1].Invert().Data & ~RemainerMask);
        return result;
    }

    CONSTEXPR bool Contains(const TFixedSizeBitMask& other) const {
        forrange(i, 0, NumWords)
            if (not Words[i].Contains(other.Words[i]))
                return false;
        return true;
    }
    CONSTEXPR bool Intersects(const TFixedSizeBitMask& other) const {
        forrange(i, 0, NumWords)
            if (Words[i].Intersects(other.Words[i]))
                return true;
        return false;
    }

    CONSTEXPR bool operator ==(const TFixedSizeBitMask& other) const {
        forrange(i, 0, NumWords)
            if (Words[i].Data != other.Words[i].Data)
                return false;
        return true;
    }
    CONSTEXPR bool operator !=(const TFixedSizeBitMask& other) const {
        return not operator ==(other);
    }

    CONSTEXPR TFixedSizeBitMask& operator &=(const TFixedSizeBitMask& other) NOEXCEPT {
        forrange(i, 0, NumWords)
            Words[i] &= other.Words[i];
        return (*this);
    }
    CONSTEXPR TFixedSizeBitMask& operator |=(const TFixedSizeBitMask& other) NOEXCEPT {
        forrange(i, 0, NumWords)
            Words[i] |= other.Words[i];
        return (*this);
    }
    CONSTEXPR TFixedSizeBitMask& operator ^=(const TFixedSizeBitMask& other) NOEXCEPT {
        forrange(i, 0, NumWords)
            Words[i] ^= other.Words[i];
        return (*this);
    }
    CONSTEXPR TFixedSizeBitMask& operator -=(const TFixedSizeBitMask& other) NOEXCEPT {
        forrange(i, 0, NumWords)
            Words[i] -= other.Words[i];
        return (*this);
    }

    CONSTEXPR TFixedSizeBitMask operator &(const TFixedSizeBitMask& other) const NOEXCEPT {
        TFixedSizeBitMask result{ *this };
        return (result &= other);
    }
    CONSTEXPR TFixedSizeBitMask operator |(const TFixedSizeBitMask& other) const NOEXCEPT {
        TFixedSizeBitMask result{ *this };
        return (result |= other);
    }
    CONSTEXPR TFixedSizeBitMask operator ^(const TFixedSizeBitMask& other) const NOEXCEPT {
        TFixedSizeBitMask result{ *this };
        return (result ^= other);
    }
    CONSTEXPR TFixedSizeBitMask operator -(const TFixedSizeBitMask& other) const NOEXCEPT {
        TFixedSizeBitMask result{ *this };
        return (result -= other);
    }

    u32 Count() const NOEXCEPT {
        u32 count = 0;
        for (const bitmask_t& bm : Words)
            count += bm.Count();
        return count;
    }

    word_t FirstBitSet() const NOEXCEPT { // return 0 if empty or bit index + 1
        forrange(i, 0, NumWords)
            if (Words[i].Data)
                return (i * BitsPerWord + Words[i].FirstBitSet_AssumeNotEmpty()) + 1;
        return 0;
    }

    word_t PopFront(const word_t prev = 0) NOEXCEPT { // return 0 if empty or bit index + 1
        forrange(i, static_cast<u32>(prev / BitsPerWord), NumWords)
            if (Words[i].Data)
                return (i * BitsPerWord + Words[i].PopFront_AssumeNotEmpty()) + 1;
        return 0;
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
