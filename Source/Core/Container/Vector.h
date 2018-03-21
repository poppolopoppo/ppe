#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/InSituAllocator.h"
#include "Core/Container/Hash.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Memory/MemoryView.h"

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define VECTOR(_DOMAIN, T) \
    ::Core::TVector<COMMA_PROTECT(T), ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTOR_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::TVector<COMMA_PROTECT(T), THREAD_LOCAL_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTORINSITU(_DOMAIN, T, _InSituCount) \
    ::Core::TVectorInSitu<COMMA_PROTECT(T), _InSituCount, ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTORINSITU_THREAD_LOCAL(_DOMAIN, T, _InSituCount) \
    ::Core::TVectorInSitu<COMMA_PROTECT(T), _InSituCount, THREAD_LOCAL_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T) >
class TVector : _Allocator {
public:
    template <typename U, typename _OtherAllocator>
    friend class TVector;

    typedef _Allocator allocator_type;
    typedef std::allocator_traits<allocator_type> allocator_traits;

    typedef T value_type;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef typename allocator_traits::pointer pointer;
    typedef typename allocator_traits::const_pointer const_pointer;

    typedef typename allocator_traits::size_type size_type;
    typedef typename allocator_traits::difference_type difference_type;

    static_assert(std::is_same<value_type, typename allocator_traits::value_type>::value,
        "allocator value_type mismatch");

    typedef TCheckedArrayIterator<value_type> iterator;
    typedef TCheckedArrayIterator<Meta::TAddConst<value_type>> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    TVector() noexcept : _capacity(0), _size(0), _data(nullptr) {}
    ~TVector() { Assert(CheckInvariants()); clear_ReleaseMemory(); }

    explicit TVector(allocator_type&& alloc) : allocator_type(std::move(alloc)), _capacity(0), _size(0), _data(nullptr) {}
    explicit TVector(const allocator_type& alloc) : allocator_type(alloc), _capacity(0), _size(0), _data(nullptr) {}

    explicit TVector(size_type count) : TVector() { resize_AssumeEmpty(count); }
    TVector(size_type count, const_reference value) : TVector() { resize_AssumeEmpty(count, value); }
    TVector(size_type count, const allocator_type& alloc) : TVector(alloc) { resize_AssumeEmpty(count); }
    TVector(size_type count, const_reference value, const allocator_type& alloc) : TVector(alloc) { resize_AssumeEmpty(count, value); }

    TVector(const TVector& other) : TVector(allocator_traits::select_on_container_copy_construction(other)) { assign(other.begin(), other.end()); }
    TVector(const TVector& other, const allocator_type& alloc) : TVector(alloc) { assign(other.begin(), other.end()); }
    TVector& operator=(const TVector& other);

    TVector(TVector&& rvalue) noexcept : TVector(static_cast<allocator_type&&>(rvalue)) { assign(std::move(rvalue)); }
    TVector(TVector&& rvalue, const allocator_type& alloc) noexcept : TVector(alloc) { assign_rvalue_(std::move(rvalue), std::false_type()); }
    TVector& operator=(TVector&& rvalue) noexcept;

    TVector(std::initializer_list<value_type> ilist) : TVector() { assign(ilist.begin(), ilist.end()); }
    TVector(std::initializer_list<value_type> ilist, const allocator_type& alloc) : TVector(alloc) { assign(ilist.begin(), ilist.end()); }
    TVector& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    TVector(const TMemoryView<const value_type>& view) : TVector() { assign(view.begin(), view.end()); }
    TVector(const TMemoryView<const value_type>& view, const allocator_type& alloc) : TVector(alloc) { assign(view.begin(), view.end()); }
    TVector& operator=(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); return *this; }

    template <typename _OtherAllocator, typename = Meta::TEnableIf<allocator_can_steal_from<_Allocator, _OtherAllocator>::value> >
    TVector(TVector<T, _OtherAllocator>&& rvalue) : TVector() { operator =(std::move(rvalue)); }
    template <typename _OtherAllocator, typename = Meta::TEnableIf<allocator_can_steal_from<_Allocator, _OtherAllocator>::value> >
    TVector& operator =(TVector<T, _OtherAllocator>&& rvalue) {
        if (_data)
            clear_ReleaseMemory();

        auto stolen = AllocatorStealBlock(allocator_(), rvalue.allocated_block_(), rvalue.allocator_());

        _capacity = checked_cast<u32>(stolen.size());
        _size = rvalue._size;
        _data = stolen.data();
        Assert(allocated_block_() == stolen);

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
    const_iterator cend() const { return end; }

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
    void shrink_to_fit() { reserve_Exactly(_size); }

    void swap(TVector& other);
    friend void swap(TVector& lhs, TVector& rhs) { lhs.swap(rhs); }

    operator TMemoryView<Meta::TAddConst<value_type>>() const { return MakeConstView(); }

    TMemoryView<value_type> MakeView() const { return TMemoryView<value_type>(_data, _size); }
    TMemoryView<Meta::TAddConst<value_type>> MakeConstView() const { return TMemoryView<Meta::TAddConst<value_type>>(_data, _size); }

    bool CheckInvariants() const;

    bool AliasesToContainer(const_pointer p) const { return ((p >= _data) && (p <= _data + _size)); }
#if USE_CORE_CHECKEDARRAYITERATOR
    bool AliasesToContainer(const iterator& it) const { return (it >= begin() && it <= end()); }
    bool AliasesToContainer(const const_iterator& it) const { return (it >= begin() && it <= end()); }
#endif

private:
    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }

    TMemoryView<T> allocated_block_() const { return TMemoryView<T>(_data, _capacity); }

    void allocator_copy_(const allocator_type& other, std::true_type );
    void allocator_copy_(const allocator_type& other, std::false_type ) { UNUSED(other); }

    void allocator_move_(allocator_type&& rvalue, std::true_type );
    void allocator_move_(allocator_type&& rvalue, std::false_type ) { UNUSED(rvalue); }

    template <typename _It>
    void assign_(_It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    void assign_(_It first, _It last, _ItCat );

    void assign_rvalue_(TVector&& rvalue, std::true_type );
    void assign_rvalue_(TVector&& rvalue, std::false_type );

    template <typename _It>
    iterator insert_(const_iterator pos, _It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    iterator insert_(const_iterator pos, _It first, _It last, _ItCat );

    void swap_(TVector& other, std::true_type );
    void swap_(TVector& other, std::false_type );

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
void clear_ReleaseMemory(TVector<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const TVector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSitu, typename _Allocator = ALLOCATOR(Container, T) >
class TVectorInSitu :
    private TInSituAllocator<T, sizeof(T) * _InSitu, _Allocator>::storage_type
,   public TVector<T, TInSituAllocator<T, sizeof(T) * _InSitu, _Allocator> > {
public:
    static_assert(_InSitu > 0, "insitu size must be greater than 0");

    typedef TInSituAllocator<T, sizeof(T) * _InSitu, _Allocator> allocator_type;
    typedef TVector<T, allocator_type> vector_type;

    static_assert(std::is_same<allocator_type, typename vector_type::allocator_type>::value, "");

    using typename vector_type::allocator_traits;
    using typename vector_type::value_type;
    using typename vector_type::pointer;
    using typename vector_type::const_pointer;
    using typename vector_type::reference;
    using typename vector_type::const_reference;
    using typename vector_type::size_type;
    using typename vector_type::difference_type;

    using vector_type::operator [];
    using vector_type::assign;

    typedef typename allocator_type::storage_type storage_type;

    TVectorInSitu() noexcept : vector_type(allocator_type(static_cast<storage_type&>(*this))) {}

    explicit TVectorInSitu(size_type count) : TVectorInSitu() { resize_AssumeEmpty(count); }
    TVectorInSitu(size_type count, const_reference value) : TVectorInSitu() { resize_AssumeEmpty(count, value); }

    TVectorInSitu(const TVectorInSitu& other) : TVectorInSitu() { assign(other.begin(), other.end()); }
    TVectorInSitu& operator=(const TVectorInSitu& other) { vector_type::operator =(other); return *this; }

    TVectorInSitu(TVectorInSitu&& rvalue) noexcept : TVectorInSitu() { assign(std::move(rvalue)); }
    TVectorInSitu& operator=(TVectorInSitu&& rvalue) noexcept { vector_type::operator=(std::move(rvalue)); return *this; }

    TVectorInSitu(std::initializer_list<value_type> ilist) : TVectorInSitu() { assign(ilist.begin(), ilist.end()); }
    TVectorInSitu& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    TVectorInSitu(const TMemoryView<const value_type>& view) : TVectorInSitu() { assign(view.begin(), view.end()); }
    TVectorInSitu& operator=(const TMemoryView<const value_type>& view) { assign(view.begin(), view.end()); return *this; }

    ~TVectorInSitu() {}

    bool UseInSitu() const { return storage_type::Contains(vector_type::data()); }

    friend hash_t hash_value(const TVectorInSitu& vector) { return hash_value(static_cast<const vector_type&>(vector)); }
};
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
} //!namespace Core

//----------------------------------------------------------------------------
// Use TVector<T> as an inplace allocator :
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void* operator new(size_t sizeInBytes, Core::TVector<T, _Allocator>& vector) {
    Assert(sizeInBytes == sizeof(T));
    void* const p = vector.push_back_Uninitialized();
    Assert(Meta::IsAligned(std::alignment_of_v<T>, p));
    return p;
}

#include "Core/Container/Vector-inl.h"
