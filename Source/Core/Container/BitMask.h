#pragma once

#include "Core/Core.h"

#include "Core/Meta/BitCount.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBitMask {
    typedef size_t word_t;

    static constexpr word_t GOne = word_t(1);
    static constexpr word_t GAllMask = word_t(-1);
    static constexpr word_t GBitCount = (sizeof(word_t)<<3);
    static constexpr word_t GBitMask = (GBitCount - 1);

    word_t Data;

    operator word_t () const { return Data; }

    size_t Count() const { return Meta::popcnt(Data); }

    template <size_t _Index>
    bool Get() const {
        STATIC_ASSERT(_Index < GBitCount);
        return (0 != (Data & (word_t(1) << _Index)));
    }

    bool Get(size_t index) const { Assert(index < GBitCount); return ((Data & (GOne<<index)) != 0); }
    void Set(size_t index, bool value) { Assert(index < GBitCount); Data = (value ? Data|(GOne<<index) : Data&~(GOne<<index)); }

    void SetTrue(size_t index) { Assert(index < GBitCount); Data |= GOne<<index; }
    void SetFalse(size_t index) { Assert(index < GBitCount); Data &= ~(GOne<<index); }

    bool operator [](size_t index) const { return Get(index); }

    bool AllTrue() const { return ((Data & GAllMask) == GAllMask); }
    bool AllFalse() const { return ((Data & GAllMask) == 0); }

    bool AnyTrue() const { return (Data != 0); }
    bool AnyFalse() const { return (Data != GAllMask); }

    void ResetAll(bool value) { Data = (value ? GAllMask : 0); }

    FBitMask Invert() const { return FBitMask{ ~Data }; }

    FBitMask operator &(FBitMask other) const { return FBitMask{ Data & other.Data }; }
    FBitMask operator |(FBitMask other) const { return FBitMask{ Data | other.Data }; }
    FBitMask operator ^(FBitMask other) const { return FBitMask{ Data ^ other.Data }; }

    FBitMask operator <<(word_t lshift) const { return FBitMask{ Data << lshift }; }
    FBitMask operator >>(word_t rshift) const { return FBitMask{ Data >> rshift }; }

    word_t FirstBitSet_AssumeNotEmpty() const { return Meta::tzcnt(Data); }
    word_t LastBitSet_AssumeNotEmpty() const { return Meta::lzcnt(Data); }

    word_t PopFront() { // return 0 if empty of (LSB index + 1)
        const size_t front = (Data ? Meta::tzcnt(Data) : INDEX_NONE);
        Data &= ~(GOne<<front); // when empty : 1 << INDEX_NONE = 1 << 0xFFFFFFFF = 0
        return (front + 1);
    }

    word_t PopFront_AssumeNotEmpty() {
        const size_t front = Meta::tzcnt(Data);
        Data &= ~(GOne<<front); // when empty : 1 << INDEX_NONE = 1 << 0xFFFFFFFF = 0
        return front;
    }

    word_t PopBack() { // return 0 if empty of (MSB index + 1)
        const size_t back = (Data ? Meta::lzcnt(Data) : INDEX_NONE);
        Data &= ~(GOne<<back); // when empty : 1 << INDEX_NONE = 1 << 0xFFFFFFFF = 0
        return (back + 1);
    }

    template <typename _Func> // LSB to MSB order
    void ForEach(_Func&& each_index) const {
        if (AllFalse()) return;
        for (word_t w = Data; w; ) {
            const word_t index = Meta::tzcnt(w);
            w &= ~(GOne<<index);
            each_index(index);
        }
    }

    template <typename _Func> // MSB to LSB order
    void ReverseForEach(_Func&& each_index) const {
        if (AllFalse()) return;
        for (word_t w = Data; w; ) {
            const word_t index = Meta::lzcnt(w);
            w &= ~(GOne<<index);
            each_index(index);
        }
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
