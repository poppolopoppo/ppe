#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Memory/UniqueView.h"
#include "Core/Meta/BitCount.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_BITSET(_NAME, _COUNT) \
    MALLOCA(Core::BitSet::word_t, CONCAT(CONCAT(_, _NAME), CONCAT(_Alloca, __LINE__)), Core::BitSet::WordCapacity(_COUNT)); \
    Core::BitSet _NAME(CONCAT(CONCAT(_, _NAME), CONCAT(_Alloca, __LINE__)).RawData, _COUNT)
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

    void CopyTo(BitSet* other) const;

    static size_t WordCapacity(size_t size) { return ((size + WordBitCount - 1) / WordBitCount); }

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
} //!namespace Core
