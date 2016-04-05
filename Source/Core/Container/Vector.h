#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/InSituAllocator.h"
#include "Core/Container/Hash.h"
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
    ::Core::Vector<COMMA_PROTECT(T), ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTOR_THREAD_LOCAL(_DOMAIN, T) \
    ::Core::Vector<COMMA_PROTECT(T), THREAD_LOCAL_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTORINSITU(_DOMAIN, T, _InSituCount) \
    ::Core::VectorInSitu<COMMA_PROTECT(T), _InSituCount, ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
#define VECTORINSITU_THREAD_LOCAL(_DOMAIN, T, _InSituCount) \
    ::Core::VectorInSitu<COMMA_PROTECT(T), _InSituCount, THREAD_LOCAL_ALLOCATOR(_DOMAIN, COMMA_PROTECT(T)) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = ALLOCATOR(Container, T) >
class Vector : _Allocator {
public:
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

    template <typename _Value, typename _Ptr, typename _Ref>
    class iterator_ : public std::iterator<std::random_access_iterator_tag, _Value, difference_type, _Ptr, _Ref> {
    public:
        typedef std::iterator<std::random_access_iterator_tag, _Value, difference_type, _Ptr, _Ref> parent_type;

        using typename parent_type::value_type;
        using typename parent_type::reference;
        using typename parent_type::pointer;
        using typename parent_type::difference_type;
        using typename parent_type::iterator_category;

        static_assert(std::is_same<typename parent_type::difference_type, typename Vector::difference_type>::value, "");

        iterator_() noexcept : _p(nullptr) {}
        explicit iterator_(pointer p) : _p(p) {}

        template <typename _V, typename _P, typename _R>
        iterator_(const iterator_<_V, _P, _R>& other) : _p(other.data()) {}
        template <typename _V, typename _P, typename _R>
        iterator_& operator=(const iterator_<_V, _P, _R>& other) { _p = other.data(); return *this; }

        pointer data() const { return _p; }

        iterator_& operator++() /* prefix */ { ++_p; return *this; }
        iterator_& operator--() /* prefix */ { --_p; return *this; }

        iterator_ operator++(int) /* postfix */ { return iterator_(_p++); }
        iterator_ operator--(int) /* postfix */ { return iterator_(_p--); }

        iterator_& operator+=(difference_type n) { _p += n; return *this; }
        iterator_& operator-=(difference_type n) { _p -= n; return *this; }

        iterator_ operator+(difference_type n) { return iterator_(_p + n); }
        iterator_ operator-(difference_type n) { return iterator_(_p - n); }

        reference operator*() const { Assert(_p); return *_p; }
        pointer operator->() const { Assert(_p); return _p; }

        reference operator[](difference_type n) const { return _p[n]; }

        bool AliasesToContainer(const Vector& v) const { return v.AliasesToContainer(_p); }

        template <typename _V, typename _P, typename _R>
        difference_type operator-(const iterator_<_V, _P, _R>& other) const { return checked_cast<difference_type>(_p - other.data()); }

        template <typename _V, typename _P, typename _R>
        bool operator==(const iterator_<_V, _P, _R>& other) const { return (_p == other.data()); }
        template <typename _V, typename _P, typename _R>
        bool operator!=(const iterator_<_V, _P, _R>& other) const { return (_p != other.data()); }

        template <typename _V, typename _P, typename _R>
        bool operator< (const iterator_<_V, _P, _R>& other) const { return (_p <  other.data()); }
        template <typename _V, typename _P, typename _R>
        bool operator> (const iterator_<_V, _P, _R>& other) const { return (_p >  other.data()); }

        template <typename _V, typename _P, typename _R>
        bool operator<=(const iterator_<_V, _P, _R>& other) const { return (_p <= other.data()); }
        template <typename _V, typename _P, typename _R>
        bool operator>=(const iterator_<_V, _P, _R>& other) const { return (_p >= other.data()); }

    private:
        pointer _p;
    };

    typedef iterator_<value_type, pointer, reference> iterator;
    typedef iterator_<const value_type, const_pointer, const_reference> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    Vector() noexcept : _data(nullptr), _size(0), _capacity(0) {}
    ~Vector() { Assert(CheckInvariants()); clear_ReleaseMemory(); }

    explicit Vector(allocator_type&& alloc) : allocator_type(std::move(alloc)), _data(nullptr), _size(0), _capacity(0) {}
    explicit Vector(const allocator_type& alloc) : allocator_type(alloc), _data(nullptr), _size(0), _capacity(0) {}

    explicit Vector(size_type count) : Vector() { resize_AssumeEmpty(count); }
    Vector(size_type count, const_reference value) : Vector() { resize_AssumeEmpty(count, value); }
    Vector(size_type count, const allocator_type& alloc) : Vector(alloc) { resize_AssumeEmpty(count); }
    Vector(size_type count, const_reference value, const allocator_type& alloc) : Vector(alloc) { resize_AssumeEmpty(count, value); }

    Vector(const Vector& other) : Vector(allocator_traits::select_on_container_copy_construction(other)) { assign(other.begin(), other.end()); }
    Vector(const Vector& other, const allocator_type& alloc) : Vector(alloc) { assign(other.begin(), other.end()); }
    Vector& operator=(const Vector& other);

    Vector(Vector&& rvalue) noexcept : Vector(static_cast<allocator_type&&>(rvalue)) { assign(std::move(rvalue)); }
    Vector(Vector&& rvalue, const allocator_type& alloc) noexcept : Vector(alloc) { assign_rvalue_(std::move(rvalue), std::false_type()); }
    Vector& operator=(Vector&& rvalue) noexcept;

    Vector(std::initializer_list<value_type> ilist) : Vector() { assign(ilist.begin(), ilist.end()); }
    Vector(std::initializer_list<value_type> ilist, const allocator_type& alloc) : Vector(alloc) { assign(ilist.begin(), ilist.end()); }
    Vector& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    template <typename U>
    explicit Vector(const MemoryView<U>& view) : Vector() { assign(view); }
    template <typename U>
    Vector(const MemoryView<U>& view, const allocator_type& alloc) : Vector(alloc) { assign(view); }
    template <typename U>
    Vector& operator=(const MemoryView<U>& view) { assign(view); return *this; }

    template <typename _It>
    Vector(_It first, _It last) : Vector() { assign(first, last); }
    template <typename _It>
    Vector(_It first, _It last, const allocator_type& alloc) : Vector(alloc) { assign(first, last); }

    size_type size() const { return _size; }
    size_type capacity() const { return _capacity; }
    size_type max_size() const { return std::numeric_limits<size_type>::max(); }
    bool empty() const { return (0 == _size); }

    pointer data() { return _data; }
    const_pointer data() const { return _data; }

    iterator begin() { return iterator(_data); }
    iterator end() { return iterator(_data + _size); }

    const_iterator begin() const { return const_iterator(_data); }
    const_iterator end() const { return const_iterator(_data + _size); }

    const_iterator cbegin() const { return const_iterator(_data); }
    const_iterator cend() const { return const_iterator(_data + _size); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    allocator_type get_allocator() const { return static_cast<const allocator_type&>(*this); }

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
    void assign(Vector&& rvalue);

    template <typename U>
    void assign(const MemoryView<U>& view) { assign(view.begin(), view.end()); }
    void assign(const MemoryView<value_type>& view) { assign(std::make_move_iterator(view.begin()), std::make_move_iterator(view.end())); }
    void assign(const MemoryView<const value_type>& view) { assign(view.begin(), view.end()); }

    template <class... _Args>
    iterator emplace(const_iterator pos, _Args&&... args);
    template <class... _Args>
    void emplace_back(_Args&&... args);
    void emplace_back(const T& value);
    void emplace_back(T&& rvalue);

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
    void pop_back();

    void clear();
    void clear_ReleaseMemory();
    void reserve(size_type count);
    void reserve_AtLeast(size_type count);
    void reserve_Additional(size_type count) { reserve_AtLeast(_size + count); }
    void reserve_AssumeEmpty(size_type count);
    void reserve_Exactly(size_type count);
    void resize(size_type count);
    void resize(size_type count, const_reference value);
    void resize_AssumeEmpty(size_type count);
    void resize_AssumeEmpty(size_type count, const_reference value);
    void shrink_to_fit() { reserve_Exactly(_size); }

    void swap(Vector& other);
    friend void swap(Vector& lhs, Vector& rhs) { lhs.swap(rhs); }

    MemoryView<value_type> MakeView() const { return MemoryView<value_type>(_data, _size); }
    MemoryView<typename std::add_const<value_type>::type> MakeConstView() const { return MemoryView<typename std::add_const<value_type>::type>(_data, _size); }

    bool CheckInvariants() const;
    bool AliasesToContainer(const_pointer p) const { return ((p >= _data) && (p <= _data + _size)); }

private:
    allocator_type& allocator_() { return static_cast<allocator_type&>(*this); }
    const allocator_type& allocator_() const { return static_cast<const allocator_type&>(*this); }

    void allocator_copy_(const allocator_type& other, std::true_type );
    void allocator_copy_(const allocator_type& other, std::false_type ) { UNUSED(other); }

    void allocator_move_(allocator_type&& rvalue, std::true_type );
    void allocator_move_(allocator_type&& rvalue, std::false_type ) { UNUSED(rvalue); }

    template <typename _It>
    void assign_(_It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    void assign_(_It first, _It last, _ItCat );

    void assign_rvalue_(Vector&& rvalue, std::true_type );
    void assign_rvalue_(Vector&& rvalue, std::false_type );

    template <typename _It>
    iterator insert_(const_iterator pos, _It first, _It last, std::input_iterator_tag );
    template <typename _It, typename _ItCat>
    iterator insert_(const_iterator pos, _It first, _It last, _ItCat );

    void swap_(Vector& other, std::true_type );
    void swap_(Vector& other, std::false_type );

    pointer _data;
    size_type _size;
    size_type _capacity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Append(Vector<T, _Allocator>& v, const MemoryView<const T>& elts);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
typename Vector<T, _Allocator>::const_iterator FindFirstOf(const Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Contains(const Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool FindElementIndexIFP(size_t *pIndex, Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Add_AssertUnique(Vector<T, _Allocator>& v, T&& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Remove_AssertExists(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Remove_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
bool Remove_ReturnIfExists_DontPreserveOrder(Vector<T, _Allocator>& v, const T& elt);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, size_t index);
//----------------------------------------------------------------------------
// Fast erase : swap last elem with elem to erase and pop_back() the vector
template <typename T, typename _Allocator>
void Erase_DontPreserveOrder(Vector<T, _Allocator>& v, const typename Vector<T, _Allocator>::const_iterator& it);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
void Clear_ReleaseMemory(Vector<T, _Allocator>& v);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
hash_t hash_value(const Vector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSitu, typename _Allocator = ALLOCATOR(Container, T) >
class VectorInSitu :
    private InSituAllocator<T, sizeof(T) * _InSitu, _Allocator>::storage_type
,   public Vector<T, InSituAllocator<T, sizeof(T) * _InSitu, _Allocator> > {
public:
    static_assert(_InSitu > 0, "insitu size must be greater than 0");

    typedef InSituAllocator<T, sizeof(T) * _InSitu, _Allocator> allocator_type;
    typedef Vector<T, allocator_type> vector_type;

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

    VectorInSitu() noexcept : vector_type(allocator_type(static_cast<storage_type&>(*this))) {}

    explicit VectorInSitu(size_type count) : VectorInSitu() { resize_AssumeEmpty(count); }
    VectorInSitu(size_type count, const_reference value) : VectorInSitu() { resize_AssumeEmpty(count, value); }

    VectorInSitu(const VectorInSitu& other) : VectorInSitu() { assign(other.begin(), other.end()); }
    VectorInSitu& operator=(const VectorInSitu& other) { vector_type::operator =(other); return *this; }

    VectorInSitu(VectorInSitu&& rvalue) noexcept : VectorInSitu() { assign(std::move(rvalue)); }
    VectorInSitu& operator=(VectorInSitu&& rvalue) noexcept { vector_type::operator=(std::move(rvalue)); return *this; }

    VectorInSitu(std::initializer_list<value_type> ilist) : VectorInSitu() { assign(ilist.begin(), ilist.end()); }
    VectorInSitu& operator=(std::initializer_list<value_type> ilist) { assign(ilist.begin(), ilist.end()); return *this; }

    ~VectorInSitu() {}

    bool UseInSitu() const { return storage_type::Contains(vector_type::data()); }

    friend hash_t hash_value(const VectorInSitu& vector) { return hash_value(static_cast<const vector_type&>(vector)); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Allocator, typename _Char, typename _Traits >
std::basic_ostream<_Char, _Traits>& operator <<(std::basic_ostream<_Char, _Traits>& oss, const Core::Vector<T, _Allocator>& vector);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Container/Vector-inl.h"
