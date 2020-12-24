#pragma once

#include "Core.h"

#include "Allocator/Allocation.h"
#include "Allocator/InSituAllocator.h"
#include "Container/Hash.h"
#include "HAL/PlatformMemory.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryView.h"
#include "Meta/Iterator.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Uses the default allocator
#define VECTOR(_DOMAIN, T) \
    ::PPE::TVector<COMMA_PROTECT(T), ALLOCATOR(_DOMAIN)>
//----------------------------------------------------------------------------
// Allocates minimum N elements, useful to avoid small allocations and fragmentation
#define VECTORMINSIZE(_DOMAIN, T, N) \
    ::PPE::TVector<COMMA_PROTECT(T), ALLOCATOR_MINSIZE(_DOMAIN, COMMA_PROTECT(T), N) >
//----------------------------------------------------------------------------
// Don't allocate for first N elements, use inline storage instead
#define VECTORINSITU(_DOMAIN, T, N) \
    ::PPE::TVector<COMMA_PROTECT(T), INLINE_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T), N) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container) >
class TVector : private _Allocator {
public:
    template <typename U, typename _OtherAllocator>
    friend class TVector;

    typedef _Allocator allocator_type;
    typedef TAllocatorTraits<allocator_type> allocator_traits;

    typedef T value_type;

    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    using size_type = size_t;
    using difference_type = ptrdiff_t;

    typedef TCheckedArrayIterator<value_type> iterator;
    typedef TCheckedArrayIterator<Meta::TAddConst<value_type>> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    TVector() NOEXCEPT : _capacity(0), _size(0), _data(nullptr) {}
    ~TVector() {
        Assert(CheckInvariants());
        clear_ReleaseMemory();
    }

    explicit TVector(Meta::FForceInit) NOEXCEPT
        : allocator_type(Meta::MakeForceInit<allocator_type>()) // used for non default-constructible allocators
        , _capacity(0), _size(0), _data(nullptr)
    {}

    explicit TVector(allocator_type&& alloc) : allocator_type(std::move(alloc)), _capacity(0), _size(0), _data(nullptr) {}
    explicit TVector(const allocator_type& alloc) : allocator_type(alloc), _capacity(0), _size(0), _data(nullptr) {}

    explicit TVector(size_type count) : TVector() { resize_AssumeEmpty(count); }
    TVector(size_type count, const_reference value) : TVector() { resize_AssumeEmpty(count, value); }
    TVector(size_type count, const allocator_type& alloc) : TVector(alloc) { resize_AssumeEmpty(count); }
    TVector(size_type count, const_reference value, const allocator_type& alloc) : TVector(alloc) { resize_AssumeEmpty(count, value); }

    TVector(const TVector& other) : TVector(allocator_traits::SelectOnCopy(other)) { assign(other.begin(), other.end()); }
    TVector(const TVector& other, const allocator_type& alloc) : TVector(alloc) { assign(other.begin(), other.end()); }
    TVector& operator=(const TVector& other);

    TVector(TVector&& rvalue) NOEXCEPT : TVector(allocator_traits::SelectOnMove(std::move(rvalue))) { assign(std::move(rvalue)); }
    TVector(TVector&& rvalue, const allocator_type& alloc) NOEXCEPT : TVector(alloc) { assign(std::move(rvalue)); }
    TVector& operator=(TVector&& rvalue) NOEXCEPT;

    TVector(std::initializer_list<value_type> ilist) : TVector() { assign(ilist.begin(), ilist.end()); }
    TVector(std::initializer_list<value_type> ilist, const allocator_type& alloc) : TVector(alloc) { assign(ilist.begin(), ilist.end()); }
    TVector& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    TVector(const TMemoryView<const value_type>& view) : TVector() { assign(view.begin(), view.end()); }
    TVector(const TMemoryView<const value_type>& view, const allocator_type& alloc) : TVector(alloc) { assign(view.begin(), view.end()); }
    TVector& operator=(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); return *this; }

    template <typename _It>
    TVector(TIterable<_It> range) : TVector() { assign(range); }
    template <typename _It>
    TVector& operator=(TIterable<_It> range) { assign(range); return *this; }

    template <typename _OtherAllocator>
    TVector(const TVector<T, _OtherAllocator>& other) : TVector() { operator =(other); }
    template <typename _OtherAllocator>
    TVector& operator =(const TVector<T, _OtherAllocator>& other) { assign(other.MakeView()); return (*this); }

    template <typename _OtherAllocator, typename = Meta::TEnableIf<has_stealallocatorblock_v<_Allocator, _OtherAllocator>> >
    TVector(TVector<T, _OtherAllocator>&& rvalue) : TVector() { operator =(std::move(rvalue)); }
    template <typename _OtherAllocator, typename = Meta::TEnableIf<has_stealallocatorblock_v<_Allocator, _OtherAllocator>> >
    TVector& operator =(TVector<T, _OtherAllocator>&& rvalue) {
        if (_data)
            clear_ReleaseMemory();

        const TMemoryView<T> b{ rvalue._data, rvalue._capacity };
        Verify(TAllocatorTraits<_OtherAllocator>::StealAndAcquire(
            &allocator_traits::Get(*this),
            TAllocatorTraits<_OtherAllocator>::Get(rvalue),
            FAllocatorBlock::From(b) ));

        _capacity = rvalue._capacity;
        _size = rvalue._size;
        _data = rvalue._data;

        rvalue._capacity = rvalue._size = 0;
        rvalue._data = nullptr;

        return (*this);
    }

    template <typename U>
    explicit TVector(const TMemoryView<U>& view) : TVector() { assign(view); }
    template <typename U>
    TVector(const TMemoryView<U>& view, const allocator_type& alloc) : TVector(alloc) { assign(view); }
    template <typename U>
    TVector& operator=(const TMemoryView<U>& view) { assign(view); return *this; }

    template <typename _It>
    TVector(_It first, _It last) : TVector() { assign(first, last); }
    template <typename _It>
    TVector(_It first, _It last, const allocator_type& alloc) : TVector(alloc) { assign(first, last); }

    size_type size() const { return _size; }
    size_type capacity() const { return _capacity; }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }
    bool empty() const { return (0 == _size); }

    pointer data() { return _data; }
    const_pointer data() const { return _data; }

    iterator begin() { return MakeCheckedIterator(_data, _size, 0); }
    iterator end() { return MakeCheckedIterator(_data, _size, _size); }

    const_iterator begin() const { return MakeCheckedIterator((const_pointer)_data, _size, 0); }
    const_iterator end() const { return MakeCheckedIterator((const_pointer)_data, _size, _size); }

    const_iterator cbegin() const { return begin(); }
    const_iterator cend() const { return end(); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    const allocator_type& get_allocator() const { return static_cast<const allocator_type&>(*this); }

    reference at(size_type pos) { Assert(pos < _size); return _data[pos]; }
    const_reference at(size_type pos) const { Assert(pos < _size); return _data[pos]; }

    reference operator[](size_type pos) { return at(pos); }
    const_reference operator[](size_type pos) const { return at(pos); }

    reference front() { return at(0); }
    const_reference front() const { return at(0); }

    reference back() { return at(_size - 1); }
    const_reference back() const { return at(_size - 1); }

    template <typename _It>
    typename std::enable_if<Meta::is_iterator<_It>::value>::type
        assign(_It first, _It last);
    void assign(size_type count, const T& value);
    void assign(std::initializer_list<T> ilist) { assign(ilist.begin(), ilist.end()); }
    void assign(TVector&& rvalue);

    template <typename _It>
    auto assign(TIterable<_It> range) { return assign(range.begin(), range.end()); }

    template <typename U>
    void assign(const TMemoryView<U>& view) { assign(view.begin(), view.end()); }
    void assign(const TMemoryView<value_type>& view) { assign(std::make_move_iterator(view.begin()), std::make_move_iterator(view.end())); }
    void assign(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); }

    template <class... _Args>
    iterator emplace(const_iterator pos, _Args&&... args);

    template <class... _Args>
    void emplace_back(_Args&&... args);
    void emplace_back(const T& value);
    void emplace_back(T&& rvalue);

    template <class... _Args>
    void emplace_back_AssumeNoGrow(_Args&&... args);
    void emplace_back_AssumeNoGrow(const T& value);
    void emplace_back_AssumeNoGrow(T&& rvalue);

    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);
    void erase_DontPreserveOrder(const_iterator pos);

    template <typename _It>
    typename std::enable_if<Meta::is_iterator<_It>::value, iterator>::type
        insert(const_iterator pos, _It first, _It last);
    iterator insert(const_iterator pos, const T& value) { return emplace(pos, value); }
    iterator insert(const_iterator pos, T&& rvalue) { return emplace(pos, std::move(rvalue)); }
    iterator insert(const_iterator pos, size_type count, const T& value);
    iterator insert(const_iterator pos, std::initializer_list<T> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

    template <typename _It>
    auto insert(const_iterator pos, TIterable<_It> range) { return insert(pos, range.begin(), range.end()); }

    void push_back(const T& value) { emplace_back(value); }
    void push_back(T&& rvalue) { emplace_back(std::move(rvalue)); }
    reference push_back_Default() { emplace_back(); return back(); }
    pointer push_back_Uninitialized() { reserve_Additional(1); return &_data[_size++]; }
    void push_back_AssumeNoGrow(const T& value);
    void push_back_AssumeNoGrow(T&& rvalue);

    void pop_back();
    value_type pop_back_ReturnBack();

    void clear();
    void clear_ReleaseMemory();
    void reserve(size_type count);
    void reserve_AtLeast(size_type count);
    void reserve_Additional(size_type count) { reserve_AtLeast(_size + count); }
    void reserve_AssumeEmpty(size_type count);
    void reserve_Exactly(size_type count);
    void resize(size_type count);
    void resize(size_type count, const_reference value);
    void resize_Uninitialized(size_type count);
    void resize_AssumeEmpty(size_type count);
    void resize_AssumeEmpty(size_type count, const_reference value);
    void shrink_to_fit();

    void swap(TVector& other) NOEXCEPT;
    friend void swap(TVector& lhs, TVector& rhs) NOEXCEPT { lhs.swap(rhs); }

    operator TMemoryView<Meta::TAddConst<value_type>>() const { return MakeConstView(); }

    TMemoryView<value_type> MakeView() const { return TMemoryView<value_type>(_data, _size); }
    TMemoryView<Meta::TAddConst<value_type>> MakeConstView() const { return TMemoryView<Meta::TAddConst<value_type>>(_data, _size); }

    bool CheckInvariants() const;

    bool AliasesToContainer(const_reference v) const { return ((&v >= _data) && (&v < _data + _size)); }
    bool AliasesToContainer(const iterator& it) const { return (it >= begin() && it < end()); }
    bool AliasesToContainer(const const_iterator& it) const { return (it >= begin() && it < end()); }

private:
    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }

    void allocator_copy_(const allocator_type& other, std::true_type );
    void allocator_copy_(const allocator_type& other, std::false_type ) { UNUSED(other); }

    template <typename _It>
    void assign_(_It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    void assign_(_It first, _It last, _ItCat );

    template <typename _It>
    iterator insert_(const_iterator pos, _It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    iterator insert_(const_iterator pos, _It first, _It last, _ItCat );

    void swap_(TVector& other, std::true_type  ) NOEXCEPT;
    void swap_(TVector& other, std::false_type ) NOEXCEPT;

    u32 _capacity;
    u32 _size;
    pointer _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(TVector<T, _Allocator>& v, const TMemoryView<const T>& elts);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _It>
void Assign(TVector<T, _Allocator>& v, _It first, _It last);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
bool Contains(const TVector<T, _Allocator>& v, const U& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename U>
size_t IndexOf(const TVector<T, _Allocator>& v, const U& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FindElementIndexIFP(size_t *pIndex, const TVector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
bool FindPredicateIndexIFP(size_t *pIndex, const TVector<T, _Allocator>& v, const _Pred& pred);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TVector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(TVector<T, _Allocator>& v, T&& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Add_Unique(TVector<T, _Allocator>& v, T&& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename... _Args>
auto Emplace_Back(TVector<T, _Allocator>& v, _Args&&... args) -> typename TVector<T, _Allocator>::iterator;
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Pred>
size_t Remove_If(TVector<T, _Allocator>& v, _Pred&& pred);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(TVector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(TVector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(TVector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists_DontPreserveOrder(TVector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TVector<T, _Allocator>& v, size_t index);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(TVector<T, _Allocator>& v, const typename TVector<T, _Allocator>::const_iterator& it);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear(TVector<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(TVector<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Reserve(TVector<T, _Allocator>& v, size_t capacity);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const TVector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FTextWriter& operator <<(FTextWriter& oss, const TVector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
FWTextWriter& operator <<(FWTextWriter& oss, const TVector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#if 0 // **BAD IDEA** : will invalidate allocated blocks upon growth => use a TSparseArray<> for this
//----------------------------------------------------------------------------
// Use TVector<T> as an in-place allocator :
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
inline void* operator new(size_t sizeInBytes, PPE::TVector<T, _Allocator>& vector) {
    Assert(sizeInBytes == sizeof(T));
    void* const p = vector.push_back_Uninitialized();
    Assert(Meta::IsAligned(std::alignment_of_v<T>, p));
    return p;
}
template <typename T, typename _Allocator>
inline void operator delete(void* ptr, PPE::TVector<T, _Allocator>& vector) {
    Assert_NoAssume(vector.AliasesToContainer(static_cast<T*>(ptr)));
    AssertNotImplemented(); // can't move elements around the vector
}
#endif

#include "Container/Vector-inl.h"
