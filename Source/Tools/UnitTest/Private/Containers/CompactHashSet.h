#pragma once

#include "Allocator/Allocation.h"
#include "Memory/MemoryView.h"

#include <algorithm>
#include <bitset>

// #TODO : very buggy, should be deleted

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <
    typename _Key
,   typename _Hash = Meta::THash<_Key>
,   typename _EqualTo = Meta::TEqualTo<_Key>
,   typename _Allocator = ALLOCATOR(Container, _Key)
>   class TCompactHashSet : _Allocator {
public:
    typedef _Key value_type;

    typedef _Hash hasher;
    typedef _EqualTo key_equal;

    typedef _Allocator allocator_type;
    typedef std::allocator_traits<_Allocator> allocator_traits;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef Meta::TAddPointer<value_type> pointer;
    typedef Meta::TAddPointer<const value_type> const_pointer;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    static constexpr size_type MaxLoadFactor = 50;

    template <typename T>
    struct TIterator {
        T* Ptr;
        size_t Capacity;
        size_t Index;

        TIterator(T* ptr, size_t capacity, size_t index)
            : Ptr(ptr)
            , Capacity(capacity)
            , Index(index) {
            Assert(Ptr);
            Assert(Index <= Capacity);
        }

        TIterator& operator++() { return Advance(); }
        TIterator& operator++(int) { TIterator tmp(*this); Advance(); return tmp; }

        T& operator *() const { Assert(Index < Capacity); return (Ptr[Index]); }
        T* operator ->() const { Assert(Index < Capacity); return (Ptr + Index); }

        inline friend bool operator ==(const TIterator& lhs, const TIterator& rhs) {
            return (lhs.Ptr == rhs.Ptr && lhs.Index == rhs.Index);
        }
        inline friend bool operator !=(const TIterator& lhs, const TIterator& rhs) {
            return (not operator ==(lhs, rhs));
        }

        TIterator& Advance(size_t amount = 1) {
            Assert(Index < Capacity);
            const value_type empty_key{};
            for (Index += amount; Index < Capacity && empty_key == Ptr[Index]; ++Index);
            return (*this);
        }
    };

    typedef TIterator<const value_type> const_iterator;

    TCompactHashSet() : _values(nullptr), _capacity(0), _size(0) {}
    ~TCompactHashSet() { clear(); }

    TCompactHashSet(const TCompactHashSet& other)
        : TCompactHashSet() {
        if (other.size()) {
            reserve(other.size());
            for (const auto& it : other)
                insert(it);
        }
    }

    TCompactHashSet& operator =(const TCompactHashSet& other) {
        clear();
        if (other.size()) {
            reserve(other.size());
            for (const auto& it : other)
                insert(it);
        }
        return (*this);
    }

    TCompactHashSet(TCompactHashSet&& rvalue)
        : TCompactHashSet() {
        std::swap(_values, rvalue._values);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_size, rvalue._size);
    }

    TCompactHashSet& operator =(TCompactHashSet&& rvalue) {
        clear();
        std::swap(_values, rvalue._values);
        std::swap(_capacity, rvalue._capacity);
        std::swap(_size, rvalue._size);
        return (*this);
    }

    size_type size() const NOEXCEPT { return _size; }
    bool empty() const NOEXCEPT { return 0 == _size; }
    size_type capacity() const NOEXCEPT { return _capacity; }

    static CONSTEXPR size_t capacity_for_n(size_t n) NOEXCEPT { return Max(16ul, n + ((100ul - MaxLoadFactor)*n) / 100ul); }

    const_iterator begin() const { return const_iterator(_values, _capacity, 0).Advance(0); }
    const_iterator end() const { return const_iterator(_values, _capacity, _capacity); }

    void reserve(size_type n) {
        const size_t atleast = capacity_for_n(n);
        if (atleast > _capacity) {
            //Assert(atleast <= Primes_[31]);
            const size_type oldcapacity = _capacity;
            _capacity = Max(size_t(16), FPlatformMaths::NextPow2(atleast));
            /*forrange(i, 0, 32)
                if (atleast <= Primes_[i]) {
                    _capacity = Primes_[i];
                    break;
                }*/
            Assert(_capacity >= atleast);
            if (oldcapacity) {
                Assert(_values);
                forrange(i, 0, oldcapacity)
                    allocator_traits::destroy(*this, _values+i);
                allocator_traits::deallocate(*this, _values, oldcapacity);
            }
            _values = allocator_traits::allocate(*this, _capacity);
            forrange(i, 0, _capacity)
                allocator_traits::construct(*this, _values+i);
        }
    }

    bool insert(const_reference value) {
        if (capacity_for_n(_size + 1) > _capacity)
            reserve(_size + 1);

        Assert(0 < _capacity);
        Assert(_size < _capacity);

        const size_t h = hasher()(value);
        /*
        size_type bucket = h % _capacity;
        size_type inc = 1 + (h>>16) % Min(5, _capacity - 1);
        */
        size_type bucket = size_type(h & (_capacity - 1));
        size_type inc = 1;// +((h >> 16) & Min(5ul, _capacity - 1));

        const value_type empty_key{};

        while (not key_equal()(_values[bucket], empty_key) &&
               not key_equal()(_values[bucket], value) )
            bucket = (bucket + inc < _capacity ? bucket + inc : (bucket + inc) - _capacity);

        if (key_equal()(_values[bucket], empty_key)) {
            _values[bucket] = value;
            _size++;
            return false;
        }
        else {
            return true;
        }
    }

    const_iterator find(const_reference value) const {
        Assert(0 < _capacity);
        Assert(_size < _capacity);
        const size_t h = hasher()(value);
        /*
        size_type bucket = h % _capacity;
        size_type inc = 1 + (h>>16) % Min(5, _capacity - 1);
        */
        size_type bucket = size_type(h & (_capacity - 1));
        size_type inc = 1;// +((h >> 16) & Min(5ul, _capacity - 1));

        const value_type empty_key{};

        while ( not key_equal()(_values[bucket], value) &&
                not key_equal()(_values[bucket], empty_key) )
            bucket = (bucket + inc < _capacity ? bucket + inc : (bucket + inc) - _capacity);

        return (not key_equal()(_values[bucket], empty_key) ? const_iterator(_values, _capacity, bucket) : end());
    }

    bool erase(const_reference value) { // bubble down
        Assert(0 < _capacity);
        Assert(_size < _capacity);
        const size_t h = hasher()(value);
        /*
        size_type bucket = h % _capacity;
        size_type inc = 1 + (h>>16) % Min(5, _capacity - 1);
        */
        size_type bucket = size_type(h & (_capacity - 1));
        size_type inc = 1;// +((h >> 16) & Min(5ul, _capacity - 1));

        const value_type empty_key{};

        while ( not key_equal()(_values[bucket], value) &&
                not key_equal()(_values[bucket], empty_key) )
            bucket = (bucket + inc < _capacity ? bucket + inc : (bucket + inc) - _capacity);

        if (key_equal()(_values[bucket], empty_key))
            return false;

        const size_type todelete = bucket;

        size_type chaintail;
        do {
            chaintail = bucket;
            bucket = (bucket + inc < _capacity ? bucket + inc : (bucket + inc) - _capacity);
        } while (not key_equal()(_values[bucket], empty_key));

        if (todelete != chaintail)
            _values[todelete] = _values[chaintail];

        _values[chaintail] = empty_key;

        _size--;

        return true;
    }

    void clear() {
        if (_capacity) {
            Assert(_values);

            forrange(i, 0, _capacity)
                allocator_traits::destroy(*this, _values+i);

            allocator_traits::deallocate(*this, _values, _capacity);

            _values = nullptr;
            _capacity = _size = 0;
        }
    }

private:
    /*
    static constexpr size_type Primes_[32] = {
       0x00000000,0x00000003,0x0000000b,0x00000017,0x00000035,0x00000061,0x000000c1,0x00000185,
       0x00000301,0x00000607,0x00000c07,0x00001807,0x00003001,0x00006011,0x0000c005,0x0001800d,
       0x00030005,0x00060019,0x000c0001,0x00180005,0x0030000b,0x0060000d,0x00c00005,0x01800013,
       0x03000005,0x06000017,0x0c000013,0x18000005,0x30000059,0x60000005,0xc0000001,0xfffffffb,
    };
    */

    pointer _values;

    size_type _capacity;
    size_type _size;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
