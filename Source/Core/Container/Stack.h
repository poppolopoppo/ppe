#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

#include <iterator>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_STACK(T, _NAME, _COUNT) \
    MALLOCA(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    Core::PodStack<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_STACK(T, _NAME, _COUNT) \
    MALLOCA(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    Core::Stack<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _IsPod = std::is_pod<T>::value >
class Stack {
public:
    typedef T value_type;

    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef pointer iterator;
    typedef const_pointer const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef typename std::random_access_iterator_tag iterator_category;

    Stack();
    Stack(pointer storage, size_type capacity);
    explicit Stack(const MemoryView<T>& storage);
    ~Stack() { clear(); }

    size_type capacity() const { return _capacity; }
    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }

    iterator begin() { return _storage; }
    iterator end() { return _storage + _size; }

    const_iterator begin() const { return _storage; }
    const_iterator end() const { return _storage + _size; }

    reverse_iterator rbegin() { return reverse_iterator(begin()); }
    reverse_iterator rend() { return reverse_iterator(end()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(begin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(end()); }

    reference at(size_type index) { Assert(index < _size); return _storage[index]; }
    const_reference at(size_type index) const { Assert(index < _size); return _storage[index]; }

    reference operator [](size_type index) { Assert(index < _size); return _storage[index]; }
    const_reference operator [](size_type index) const { Assert(index < _size); return _storage[index]; }

    MemoryView<T> MakeView() { return MemoryView<T>(_storage, _size); }
    MemoryView<const T> MakeView() const { return MemoryView<const T>(_storage, _size); }

    template <typename _Arg0, typename... _Args>
    void Push(_Arg0&& arg0, _Args&&... args);
    bool Pop(pointer pvalue);

    pointer Peek() { return ((0 == _size) ? nullptr : &_storage[_size - 1] ); }
    const_pointer Peek() const { return ((0 == _size) ? nullptr : &_storage[_size - 1] ); }

    void clear();

    // used to manipulate the stack as an allocator :
    pointer Allocate(size_type count);
    bool DeallocateIFP(pointer p, size_type count);
    void Deallocate_AssertLIFO(pointer p, size_type count);

    void Swap(Stack& other);

private:
    size_type _size;
    size_type _capacity;
    pointer _storage;
};
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
Stack<T, _IsPod>::Stack()
:   _size(0), _capacity(0), _storage(nullptr) {}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
Stack<T, _IsPod>::Stack(pointer storage, size_type capacity)
:   _size(0), _capacity(capacity), _storage(storage) {
    Assert(0 == _capacity || _storage);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
Stack<T, _IsPod>::Stack(const MemoryView<T>& storage)
:   Stack(storage.Pointer(), storage.size()) {}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
void Stack<T, _IsPod>::Push(_Arg0&& arg0, _Args&&... args) {
    Assert(_storage);
    Assert(_size < _capacity);

    new ((void*)&_storage[_size++]) T{ std::forward<_Arg0>(arg0), std::forward<_Args>(args)... };
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
bool Stack<T, _IsPod>::Pop(pointer pvalue) {
    Assert(pvalue);

    if (0 == _size)
        return false;

    Assert(_storage);

    T& elt = _storage[--_size];
    if (pvalue)
        *pvalue = std::move(elt);
    if (false == _IsPod)
        elt.~T();

    return true;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
auto Stack<T, _IsPod>::Allocate(size_type count) -> pointer {
    if (_size + count > _capacity)
        return nullptr;

    Assert(_storage);

    pointer const p = &_storage[_size];
    _size += count;

    return p;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
bool Stack<T, _IsPod>::DeallocateIFP(pointer p, size_type count) {
    Assert(_size > 0);
    Assert(p >= _storage);
    Assert(p + count <= _storage + _size);

    // can only delete the last block allocated !
    if ((p + count) != (_storage + _size))
        return false;

    _size -= count;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void Stack<T, _IsPod>::Deallocate_AssertLIFO(pointer p, size_type count) {
    if (false == DeallocateIFP(p, count))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void Stack<T, _IsPod>::clear() {
    if (false == _IsPod) {
        for (size_t i = 0; i < _size; ++i)
            _storage[i].~T();
    }
    _size = 0;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void Stack<T, _IsPod>::Swap(Stack& other) {
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_storage, other._storage);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void swap(Stack<T, _IsPod>& lhs, Stack<T, _IsPod>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using PodStack = Stack<T, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment = std::alignment_of<T>::value >
class FixedSizeStack : public Stack<T> {
public:
    typedef Stack<T> parent_type;

    using typename parent_type::value_type;
    using typename parent_type::pointer;
    using typename parent_type::const_pointer;
    using typename parent_type::reference;
    using typename parent_type::const_reference;

    using typename parent_type::size_type;
    using typename parent_type::difference_type;

    using typename parent_type::iterator;
    using typename parent_type::const_iterator;

    using typename parent_type::reverse_iterator;
    using typename parent_type::const_reverse_iterator;

    using typename parent_type::iterator_category;

    FixedSizeStack() : parent_type(reinterpret_cast<pointer>(&_insitu), _Capacity) {}

    FixedSizeStack(FixedSizeStack&& ) = delete;
    FixedSizeStack& operator =(FixedSizeStack&& rvalue) = delete;

    FixedSizeStack(const FixedSizeStack& other) = delete;
    FixedSizeStack& operator =(const FixedSizeStack& other) = delete;

    void Swap(Stack<T>& other) = delete;

private:
    // /!\ won't call any ctor or dtor, values are considered as undefined
    typename ALIGNED_STORAGE(sizeof(T) * _Capacity, _Alignment) _insitu;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
