#include "stdafx.h"

#include "Container/BitSet.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBitSet::FBitSet(word_t *storage, size_t size)
:   _storage(storage)
,   _size(size) {
    Assert(_storage || 0 == _size);
}
//----------------------------------------------------------------------------
bool FBitSet::AllTrue() const {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    constexpr word_t allTrueMask = word_t(-1);

    forrange(i, 0, wordCount - 1) {
        if (_storage[i] != allTrueMask)
            return false;
    }

    if (_size & WordBitMask) {
        const size_t lastWordMask = (1 << (_size & WordBitMask)) - 1;
        return (lastWordMask == (_storage[wordCount - 1] & lastWordMask));
    }
    else {
        return (_storage[wordCount - 1] == allTrueMask);
    }
}
//----------------------------------------------------------------------------
bool FBitSet::AllFalse() const {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    forrange(i, 0, wordCount - 1) {
        if (_storage[i] != 0)
            return false;
    }

    if (_size & WordBitMask) {
        const size_t lastWordMask = (1 << (_size & WordBitMask)) - 1;
        return (0 == (_storage[wordCount - 1] & lastWordMask));
    }
    else {
        return (_storage[wordCount - 1] == 0);
    }
}
//----------------------------------------------------------------------------
bool FBitSet::AnyTrue() const {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    forrange(i, 0, wordCount - 1) {
        if (_storage[i] != 0)
            return true;
    }

    if (_size & WordBitMask) {
        const size_t lastWordMask = (1 << (_size & WordBitMask)) - 1;
        return (0 != (_storage[wordCount - 1] & lastWordMask));
    }
    else {
        return (_storage[wordCount] != 0);
    }
}
//----------------------------------------------------------------------------
bool FBitSet::AnyFalse() const {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    constexpr word_t allTrueMask = word_t(-1);

    forrange(i, 0, wordCount - 1)
        if (_storage[i] != allTrueMask)
            return true;

    if (_size & WordBitMask) {
        const size_t lastWordMask = (1 << (_size & WordBitMask)) - 1;
        return (lastWordMask != (_storage[wordCount - 1] & lastWordMask));
    }
    else {
        return (_storage[wordCount - 1] != allTrueMask);
    }
}
//----------------------------------------------------------------------------
void FBitSet::ResetAll(bool value) {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    if (value) {
        constexpr word_t allTrueMask = word_t(-1);
        forrange(i, 0, wordCount)
            _storage[i] = allTrueMask;
    }
    else {
        constexpr word_t allFalseMask = word_t(0);
        forrange(i, 0, wordCount)
            _storage[i] = allFalseMask;
    }
}
//----------------------------------------------------------------------------
void FBitSet::CopyTo(FBitSet* other) const {
    Assert(other);
    Assert(other->_size == _size);

    const size_t wordCount = WordCapacity(_size);
    forrange(i, 0, wordCount)
        other->_storage[i] = _storage[i];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE