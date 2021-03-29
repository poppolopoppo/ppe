#pragma once

#include "Core_fwd.h"

#include "Allocator/Alloca.h"
#include "Container/BitMask.h"

#include <atomic>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Word, bool _NeedAtomic = false >
struct TBitTree {
    using word_t = Meta::TConditional<_NeedAtomic, std::atomic<_Word>, _Word>;
    using mask_t = TBitMask<_Word>;
    static CONSTEXPR u32 WordBitCount = mask_t::BitCount;

    word_t* Bits{ nullptr };
    u32 TreeDepth{ 0 };
    u32 DesiredSize{ 0 };
    u32 LeafsNumWords{ 0 };
    u32 LeafsFirstWord{ 0 };

    CONSTEXPR u32 Capacity() const { return (LeafsNumWords * WordBitCount); }
    CONSTEXPR u32 TotalNumWords() const { return (LeafsFirstWord + LeafsNumWords); }
    CONSTEXPR size_t AllocationSize() const { return (TotalNumWords() * sizeof(word_t)); }

    bool Empty_ForAssert() const { return (CountOnes(DesiredSize) == 0); }
    bool Full() const { return mask_t{ Word(0) }.AllTrue(); }

    CONSTEXPR void SetupMemoryRequirements(u32 desiredSize) NOEXCEPT {
        u32 depth = 1;
        u32 rowSizeInWords = 1;
        u32 rowOffsetInWords = 0;
        u32 totalCapacity = WordBitCount;
        while (totalCapacity < desiredSize) {
            rowOffsetInWords += rowSizeInWords;
            rowSizeInWords *= WordBitCount;
            totalCapacity *= WordBitCount;
            depth++;
        }

        TreeDepth = depth;
        DesiredSize = desiredSize;
        LeafsNumWords = ((DesiredSize + WordBitCount - 1) / WordBitCount); // clamp leafs to desired size
        LeafsFirstWord = rowOffsetInWords;
    }

    CONSTEXPR void Initialize(word_t* storage, bool enabledByDefault = false) { // must call SetupMemoryRequirements() before
        Assert(storage);
        Bits = storage;

        // initialize the bits in the array, clamped to desired size
        mask_t m;
        m.ResetAll(enabledByDefault);
        for (auto* w = Bits, *e = w + TotalNumWords(); w != e; ++w)
            *w = m;

        if (not enabledByDefault) {
            u32 width = 1;
            for (u32 d = 1; d < TreeDepth; d++)
                width *= WordBitCount;
            width /= WordBitCount;
            u32 offset = LeafsFirstWord - width;
            u32 granularity = WordBitCount;

            for (int d = TreeDepth - 2; d >= 0; d--) {
                u32 trueBits = (width * WordBitCount - (DesiredSize + granularity - 1) / granularity);
                const u32 trueWords = (trueBits / WordBitCount);
                trueBits %= WordBitCount;

                m.ResetAll(true);
                for (u32 fill = width - trueWords; fill < width; fill++)
                    Word(offset + fill) = m;

                if (trueBits)
                    Word(offset + width - trueWords - 1) = mask_t::UnsetFirstN(WordBitCount - trueBits);

                width = width / WordBitCount;
                offset -= width;
                granularity *= WordBitCount;
            }

            if (DesiredSize % WordBitCount)
                Word(TotalNumWords() - 1) = mask_t::UnsetFirstN(DesiredSize % WordBitCount);
        }
    }

    word_t& Word(u32 at) NOEXCEPT {
        Assert(at < TotalNumWords());
        return Bits[at];
    }
    const word_t& Word(u32 at) const NOEXCEPT {
        Assert(at < TotalNumWords());
        return Bits[at];
    }

    u32 CountOnes(u32 upTo) const {
        Assert(upTo <= DesiredSize);
        const word_t* pword = &Word(LeafsFirstWord);

        u32 cnt = 0;
        while (upTo >= WordBitCount) {
            cnt += checked_cast<u32>(FPlatformMaths::popcnt(*pword));
            pword++;
            upTo -= WordBitCount;
        }

        if (upTo)
            cnt += checked_cast<u32>(FPlatformMaths::popcnt(*pword << (WordBitCount - upTo)));

        return cnt;
    }

    u32 NextAllocateBit() const NOEXCEPT { // returns leaf index of UINT32_MAX if full
        Assert(Bits);
        if (Unlikely(Full()))
            return UMax;

        u32 bit = 0;
        u32 offset = 0;

        for (u32 d = 0; d < TreeDepth; ++d) {
            mask_t m{ Word(offset) };
            const u32 jmp = checked_cast<u32>(m.Invert().CountTrailingZeros());
            Assert(jmp < WordBitCount);

            bit = (bit * WordBitCount + jmp);
            offset = (offset * WordBitCount + 1 + jmp);
        }

        Assert(bit < Capacity());
        Assert_NoAssume(not IsAllocated(bit));
        return bit;
    }

    u32 NextAllocateBit(u32 after) const NOEXCEPT { // returns leaf index of UINT32_MAX if full
        Assert(Bits);
        Assert(after < DesiredSize);
        if (Unlikely(Full()))
            return UINT32_MAX;

        u32 d = TreeDepth - 1;
        u32 bit = after;
        u32 r = (bit % WordBitCount);
        u32 offset = (LeafsFirstWord + bit / WordBitCount);

        Assert_NoAssume(bit < Capacity());

        mask_t m{ Word(offset) };
        if (not m.Get(r))
            return bit; // start was unallocated

        m |= mask_t::SetFirstN(r);
        if (Likely(not m.Full())) {
            const u32 jmp = m.Invert().CountTrailingZeros();
            Assert(jmp < WordBitCount);
            return (bit - r + jmp);
        }
        if (d > 0) do {
            d--;
            r = (offset - 1) % WordBitCount;
            offset = (offset - 1) / WordBitCount;

            m = mask_t{ Word(offset) };
            m |= mask_t::SetFirstN(r);
            if (Likely(not m.full())) {
                for (;;) {
                    const u32 jmp = m.Invert().CountTrailingZeros();
                    Assert(jmp < WordBitCount);
                    if (d == TreeDepth - 1) {
                        Assert(not m.Get(jmp));
                        bit = ((offset - LeafsFirstWord) * WordBitCount + jmp);
                        Assert(bit < DesiredSize);
                        return bit;
                    }

                    d++;
                    offset = (offset * WordBitCount + 1 + jmp);
                    m = mask_t{ Word(offset) };
                }
            }
        } while (d);

        return UMax;
    }

    bool IsAllocated(u32 bit) const NOEXCEPT {
        Assert(Bits);
        Assert(bit < DesiredSize);

        bit += LeafsFirstWord * WordBitCount;
        const u32 w = (bit / WordBitCount);
        const u32 r = (bit % WordBitCount);

        return mask_t{ Word(w) }.Get(r);
    }

    void AllocateBit(u32 bit) NOEXCEPT {
        Assert(Bits);
        Assert(not Full());
        Assert(bit < DesiredSize);

        mask_t m;
        u32 d = TreeDepth - 1;
        u32 r = (bit % WordBitCount);
        u32 w = (LeafsFirstWord + bit / WordBitCount);
        word_t* pword = &Word(w);

        m = { *pword };
        Assert(not m.Get(r));
        m.SetTrue(r);
        *pword = m;

        if (Unlikely((d > 0) & m.AllTrue())) {
            do {
                r = (w - 1) % WordBitCount;
                w = (w - 1) / WordBitCount;
                pword = &Word(w);

                m = { *pword };
                Assert(not m.Get(r));
                m.SetTrue(r);
                *pword = m;

                if (Likely(not m.AllTrue()))
                    break;

                d--;
            } while (d);
        }
    }

    u32 Allocate() NOEXCEPT { // returns leaf index of UMax if full
        Assert(Bits);
        if (Unlikely(Full()))
            return UMax;

        mask_t m;
        u32 d = 0;
        u32 bit = 0;
        u32 offset = 0;
        for (;;) {
            word_t* pword = &Word(offset);
            m = { *pword };
            const u32 jmp = m.Invert().CountTrailingZeros();
            Assert(jmp < WordBitCount);
            bit = bit * WordBitCount + jmp;

            if (TreeDepth - 1 == d) {
                Assert(not m.Get(jmp));
                m.SetTrue(jmp);
                *pword = m;

                if (Unlikely(d > 0 && m.Full())) do {
                    const u32 r = (offset - 1) % WordBitCount;
                    offset = (offset - 1) / WordBitCount;
                    pword = &Word(offset);

                    m = { *pword };
                    Assert(not m.Full());
                    m.SetTrue(r);
                    *pword = m;

                    if (Likely(not m.Full()))
                        break;

                    d--;
                } while (d);

                break;
            }

            d++;
            offset = offset * WordBitCount + 1 + jmp;
        }

        return bit;
    }

    void Deallocate(u32 bit) NOEXCEPT {
        Assert(Bits);
        Assert(bit < DesiredSize);

        mask_t m;
        u32 d = TreeDepth - 1;
        u32 r = (bit % WordBitCount);
        u32 offset = LeafsFirstWord + bit / WordBitCount;
        word_t* pword = &Word(offset);

        m = { *pword };
        Assert(m.Get(r));
        bool wasFull = (m.AllTrue());
        m.SetFalse(r);
        *pword = m;

        if (Unlikely(wasFull && d > 0)) do {
            r = ((offset - 1) % WordBitCount);
            offset = ((offset - 1) / WordBitCount);
            pword = &Word(offset);

            m = { *pword };
            wasFull = m.AllTrue();
            Assert(m.Get(r));
            m.SetFalse(r);
            *pword = m;

            if (Likely(not wasFull))
                break;

            d--;
        } while (d);
    }

    static CONSTEXPR u32 StaticMemoryRequirements(u32 desiredSize) {
        TBitTree stub;
        stub.SetupMemoryRequirements(desiredSize);
        return stub.TotalNumWords();
    }
};
//----------------------------------------------------------------------------
template <typename _Word>
using TAtomicBitTree = TBitTree<_Word, true>;
//----------------------------------------------------------------------------
template <typename _Word, u32 _DesiredSize, u32 _TotalNumWords, bool _NeedAtomic = false >
struct TStaticBitTree : TBitTree<_Word, _NeedAtomic> {
    using parent_t = TBitTree<_Word, _NeedAtomic>;
    using typename parent_t::word_t;
    using typename parent_t::mask_t;
    using parent_t::WordBitCount;
    using parent_t::Full;
    using parent_t::FirstAvailable;
    using parent_t::NextBit;
    using parent_t::AllocateBit;
    using parent_t::Allocate;
    using parent_t::Deallocate;
    using parent_t::TotalNumWords;

    word_t Storage[_TotalNumWords];

    CONSTEXPR TStaticBitTree() NOEXCEPT {
        parent_t::SetupMemoryRequirements(_DesiredSize);
        parent_t::Initialize(Storage, _TotalNumWords);
    }
};
//----------------------------------------------------------------------------
template <typename _Word, u32 _DesiredSize, bool _NeedAtomic = false>
using TFixedSizeBitTree = TStaticBitTree<_Word, _DesiredSize,
    TBitTree<_Word, _NeedAtomic>::StaticMemoryRequirements(_DesiredSize) >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
