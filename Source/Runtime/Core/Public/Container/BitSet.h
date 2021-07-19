#pragma once

#include "Core.h"

#include "Allocator/Alloca.h"
#include "Container/BitMask.h"

#define STACKLOCAL_BITSET(_NAME, _COUNT) \
    MALLOCA_POD(::PPE::FBitSet::word_t, CONCAT(CONCAT(_, _NAME), ANONYMIZE(_Alloca)), ::PPE::FBitSet::WordCapacity(_COUNT)); \
    ::PPE::FBitSet _NAME(CONCAT(CONCAT(_, _NAME), ANONYMIZE(_Alloca)).RawData, _COUNT)
#define STACKLOCAL_BITSET_INIT(_NAME, _COUNT, _ALL_TRUE) \
    MALLOCA_POD(::PPE::FBitSet::word_t, CONCAT(CONCAT(_, _NAME), ANONYMIZE(_Alloca)), ::PPE::FBitSet::WordCapacity(_COUNT)); \
    ::PPE::FBitSet _NAME(CONCAT(CONCAT(_, _NAME), ANONYMIZE(_Alloca)).RawData, _COUNT, _ALL_TRUE)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBitSet {
public:
    using word_t = size_t;
    using mask_t = TBitMask<word_t>;

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

    FBitSet() = default;
    FBitSet(word_t *storage, size_t size) NOEXCEPT;
    FBitSet(word_t *storage, size_t size, bool allTrue) NOEXCEPT;

    size_t size() const { return _size; }

    FORCE_INLINE bool Get(size_t index) const { return 0 != (Word_(index) & IndexFlag_(index)); }
    FORCE_INLINE void Set(size_t index, bool value) { if (value) SetTrue(index); else SetFalse(index); }

    FORCE_INLINE void SetTrue(size_t index) { Word_(index) |= IndexFlag_(index); }
    FORCE_INLINE void SetFalse(size_t index) { Word_(index) &= ~IndexFlag_(index); }

    NODISCARD FORCE_INLINE bool Unset(size_t index) {
        if (Get(index)) {
            SetFalse(index);
            return true;
        }
        return false;
    }

    FORCE_INLINE bool operator [](size_t index) const { return Get(index); }

    FORCE_INLINE word_t Word(size_t w) const {
        Assert(w < _size);
        return _storage[w];
    }

    bool AllTrue() const;
    bool AllFalse() const;

    bool AnyTrue() const;
    bool AnyFalse() const;

    void ResetAll(bool value);

    void CopyTo(FBitSet* other) const;

    word_t FirstBitSet() const NOEXCEPT; // return _size if empty
    word_t LastBitSet() const NOEXCEPT; // return _size if empty
    word_t PopFront() NOEXCEPT; // return 0 if empty or bit index + 1
    word_t PopFront_After(size_t last = 0) NOEXCEPT; // return 0 if empty or bit index + 1

    static size_t WordCapacity(size_t size) { return ((size + WordBitMask) >> WordBitShift); }

protected:
    FORCE_INLINE static constexpr size_t IndexFlag_(size_t index) { return (size_t(1) << (index & WordBitMask)); }
    FORCE_INLINE word_t& Word_(size_t index) { Assert(index < _size); return _storage[index >> WordBitShift]; }
    FORCE_INLINE const word_t& Word_(size_t index) const { Assert(index < _size); return _storage[index >> WordBitShift]; }

private:
    word_t* _storage;
    size_t _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
