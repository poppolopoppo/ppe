#include "stdafx.h"

#include "BitSet.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BitSet::BitSet(word_t *storage, size_t size)
:   _storage(storage)
,   _size(size) {
    Assert(_storage || 0 == _size);
}
//----------------------------------------------------------------------------
bool BitSet::AllTrue() const {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    static const word_t allTrueMask = word_t(-1);

    forrange(i, 0, wordCount - 1)
        if (_storage[i] != allTrueMask)
            return false;

    return _storage[wordCount - 1] == ~(allTrueMask << (_size & WordBitMask));
}
//----------------------------------------------------------------------------
bool BitSet::AllFalse() const {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    forrange(i, 0, wordCount - 1)
        if (_storage[i] != 0)
            return false;

    return _storage[wordCount - 1] == (word_t(-1) << (_size & WordBitMask));
}
//----------------------------------------------------------------------------
bool BitSet::AnyTrue() const {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    forrange(i, 0, wordCount - 1)
        if (_storage[i] != 0)
            return true;

    return 0 != (_storage[wordCount - 1] & ~(word_t(-1) << (_size & WordBitMask)));
}
//----------------------------------------------------------------------------
bool BitSet::AnyFalse() const {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    static const word_t allTrueMask = word_t(-1);

    forrange(i, 0, wordCount - 1)
        if (_storage[i] != allTrueMask)
            return true;

    return _storage[wordCount - 1] != ~(allTrueMask << (_size & WordBitMask));
}
//----------------------------------------------------------------------------
void BitSet::ResetAll(bool value) {
    const size_t wordCount = WordCapacity(_size);
    Assert(wordCount);

    if (value) {
        forrange(i, 0, wordCount)
            _storage[i] = word_t(-1);
    }
    else {
        forrange(i, 0, wordCount)
            _storage[i] = 0;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
