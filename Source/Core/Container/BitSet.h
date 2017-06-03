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
    MALLOCA(::Core::FBitSet::word_t, CONCAT(CONCAT(_, _NAME), CONCAT(_Alloca, __LINE__)), ::Core::FBitSet::WordCapacity(_COUNT)); \
    ::Core::FBitSet _NAME(CONCAT(CONCAT(_, _NAME), CONCAT(_Alloca, __LINE__)).RawData, _COUNT)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBitSet {
public:
    typedef size_t word_t;
#if     defined(ARCH_64BIT)
    STATIC_CONST_INTEGRAL(word_t, WordBitCount, 64);
    STATIC_CONST_INTEGRAL(word_t, WordBitMask,  63);
    STATIC_CONST_INTEGRAL(word_t, WordBitShift,  6);
#elif   defined(ARCH_32BIT)
    STATIC_CONST_INTEGRAL(word_t, WordBitCount, 32);
    STATIC_CONST_INTEGRAL(word_t, WordBitMask,  31);
    STATIC_CONST_INTEGRAL(word_t, WordBitShift,  5);
#else
#   error "unsupported architecture !"
#endif

    FBitSet(word_t *storage, size_t size);

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

    void CopyTo(FBitSet* other) const;

    static size_t WordCapacity(size_t size) { return ((size + WordBitMask) >> WordBitShift); }

protected:
    FORCE_INLINE static constexpr size_t IndexFlag_(size_t index) { return (size_t(1) << (index & WordBitMask)); }
    word_t& Word_(size_t index) { Assert(index < _size); return _storage[index >> WordBitShift]; }
    const word_t& Word_(size_t index) const { Assert(index < _size); return _storage[index >> WordBitShift]; }

private:
    word_t *_storage;
    size_t _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWordBitSet {
public:
    typedef size_t word_t;

    FWordBitSet(size_t data, size_t size) : _data(data), _size(size) {
        Assert(_size < MaxBit);
    }

    size_t data() const { return _data; }
    size_t size() const { return _size; }

    template <size_t _Index>
    bool Get() const {
        STATIC_ASSERT(_Index < MaxBit);
        return (0 != (_data & (size_t(1) << _Index)));
    }

    bool Get(size_t index) const { Assert(index < _size); return (0 != (_data & (size_t(1) << index))); }
    void Set(size_t index, bool value) { if (value) SetTrue(index); else SetFalse(index); }

    void SetTrue(size_t index) { Assert(index < _size); _data |= size_t(1) << index; }
    void SetFalse(size_t index) { Assert(index < _size); _data &= ~(size_t(1) << index); }

    bool operator [](size_t index) const { return Get(index); }

    bool AllTrue() const { return ((_data & AllMask_()) == AllMask_()); }
    bool AllFalse() const { return ((_data & AllMask_()) == 0); }

    bool AnyTrue() const { return ((_data & AllMask_()) != 0); }
    bool AnyFalse() const { return ((_data & AllMask_()) != AllMask_()); }

    void ResetAll(bool value) { _data = (value ? size_t(-1) : 0); }

    FWordBitSet BitAnd(const FWordBitSet& other) const {
        Assert(_size == other._size);
        return FWordBitSet(_data & other._data, _size);
    }

    FWordBitSet BitOr(const FWordBitSet& other) const {
        Assert(_size == other._size);
        return FWordBitSet(_data | other._data, _size);
    }

private:
#ifdef ARCH_X64
    static constexpr size_t MaxBit = 64;
#else
    static constexpr size_t MaxBit = 32;
#endif

    size_t AllMask_() const { return (1 << _size) - 1; }

    size_t _data;
    size_t _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
