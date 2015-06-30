#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Memory/UniqueView.h"
#include "Core/Meta/BitCount.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BitSet {
public:
    typedef size_t word_t;
    STATIC_CONST_INTEGRAL(word_t, WordBitCount, Meta::BitCount<word_t>::value);
    STATIC_CONST_INTEGRAL(word_t, WordBitMask, WordBitCount - 1);

    BitSet(word_t *storage, size_t size);

    size_t size() const { return _size; }

    bool Get(size_t index) const { return 0 != (Word_(index) & IndexFlag_(index)); }
    void Set(size_t index, bool value) { if (value) SetTrue(index); else SetFalse(index); }

    void SetTrue(size_t index) { Word_(index) |= IndexFlag_(index); }
    void SetFalse(size_t index) { Word_(index) &= ~IndexFlag_(index); }

    bool operator [](size_t index) const { return Get(index); }

    bool AllTrue() const;
    bool AllFalse() const;

    bool AnyTrue() const;
    bool AnyFalse() const;

    void ResetAll(bool value);

    static size_t WordCapacity(size_t size) { return (0 == size) ? 0 : 1 + (size / WordBitCount); }

private:
    static size_t IndexFlag_(size_t index) { return (size_t(1) << (index & WordBitMask)); }
    word_t& Word_(size_t index) { Assert(index < _size); return _storage[index / WordBitCount]; }
    const word_t& Word_(size_t index) const { Assert(index < _size); return _storage[index / WordBitCount]; }

    word_t *_storage;
    size_t _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_BITSET(_NAME, _COUNT) \
    STACKLOCAL_POD_ARRAY(BitSet::word_t, CONCAT(_NAME, _Storage), BitSet::WordCapacity(_COUNT)); \
    BitSet _NAME(CONCAT(_NAME, _Storage).Pointer(), _COUNT)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
