#pragma once

#include "Core.h"

#include "Allocator/Alloca.h"
#include "Memory/MemoryView.h"
#include "Meta/AlignedStorage.h"

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_STACK(T, _NAME, _COUNT) \
    MALLOCA_POD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TPodStack<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_ASSUMEPOD_STACK(T, _NAME, _COUNT) \
    MALLOCA_ASSUMEPOD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TPodStack<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_STACK(T, _NAME, _COUNT) \
    MALLOCA_ASSUMEPOD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TStack<T> _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_HEAP(T, _Pred, _NAME, _COUNT) \
    MALLOCA_POD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TPODStackHeapAdapter<T, Meta::TDecay<decltype(_Pred)> > _NAME( CONCAT(_Alloca_, _NAME).MakeView(), _Pred )
//----------------------------------------------------------------------------
#define STACKLOCAL_ASSUMEPOD_HEAP(T, _Pred, _NAME, _COUNT) \
    MALLOCA_ASSUMEPOD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TPODStackHeapAdapter<T, Meta::TDecay<decltype(_Pred)> > _NAME( CONCAT(_Alloca_, _NAME).MakeView(), _Pred )
//----------------------------------------------------------------------------
#define STACKLOCAL_HEAP(T, _Pred, _NAME, _COUNT) \
    MALLOCA_ASSUMEPOD(T, CONCAT(_Alloca_, _NAME), _COUNT); \
    PPE::TStackHeapAdapter<T, Meta::TDecay<decltype(_Pred)> > _NAME( CONCAT(_Alloca_, _NAME).MakeView(), _Pred )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, bool _IsPod = Meta::TIsPod<T>::value >
class TStack {
public:
    typedef T value_type;

    typedef Meta::TAddPointer<T> pointer;
    typedef Meta::TAddPointer<const T> const_pointer;
    typedef Meta::TAddReference<T> reference;
    typedef Meta::TAddReference<const T> const_reference;

    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef TCheckedArrayIterator<T> iterator;
    typedef TCheckedArrayIterator<Meta::TAddConst<T>> const_iterator;

    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    typedef typename std::random_access_iterator_tag iterator_category;

    TStack();
    TStack(pointer storage, size_type capacity);
    explicit TStack(const TMemoryView<T>& storage);
    ~TStack() { clear(); }

    size_type capacity() const { return _capacity; }
    size_type size() const { return _size; }
    bool empty() const { return 0 == _size; }
    bool full() const { return (_capacity == _size); }

    iterator begin() { return MakeCheckedIterator(_storage, _size, 0); }
    iterator end() { return MakeCheckedIterator(_storage, _size, _size); }

    const_iterator begin() const { return MakeCheckedIterator(const_pointer(_storage), _size, 0); }
    const_iterator end() const { return MakeCheckedIterator(const_pointer(_storage), _size, _size); }

    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    reference at(size_type index) { Assert(index < _size); return _storage[index]; }
    const_reference at(size_type index) const { Assert(index < _size); return _storage[index]; }

    reference operator [](size_type index) { Assert(index < _size); return _storage[index]; }
    const_reference operator [](size_type index) const { Assert(index < _size); return _storage[index]; }

    TMemoryView<T> MakeView() { return TMemoryView<T>(_storage, _size); }
    TMemoryView<const T> MakeView() const { return TMemoryView<const T>(_storage, _size); }
    TMemoryView<const T> MakeConstView() const { return TMemoryView<const T>(_storage, _size); }

    pointer Push_Uninitialized();
    template <typename _Arg0, typename... _Args>
    void Push(_Arg0&& arg0, _Args&&... args);
    bool Pop(pointer pvalue = nullptr);

    bool Contains(const_reference item) const;

    pointer Peek() { return ((0 == _size) ? nullptr : &_storage[_size - 1] ); }
    const_pointer Peek() const { return ((0 == _size) ? nullptr : &_storage[_size - 1] ); }

    void clear();

    // used to manipulate the stack as an allocator :
    pointer Allocate(size_type count);
    bool DeallocateIFP(pointer p, size_type count);
    void Deallocate_AssertLIFO(pointer p, size_type count);

    void Swap(TStack& other);

    bool AliasesToContainer(const_pointer p) const {
        return (p >= _storage && p < _storage + _size);
    }

protected:
    size_type _size;
    size_type _capacity;
    pointer _storage;
};
//----------------------------------------------------------------------------
template <typename T>
using TPodStack = TStack<T, true>;
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
TStack<T, _IsPod>::TStack()
:   _size(0), _capacity(0), _storage(nullptr) {}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
TStack<T, _IsPod>::TStack(pointer storage, size_type capacity)
:   _size(0), _capacity(capacity), _storage(storage) {
    Assert(0 == _capacity || _storage);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
TStack<T, _IsPod>::TStack(const TMemoryView<T>& storage)
:   TStack(storage.Pointer(), storage.size()) {}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
auto TStack<T, _IsPod>::Push_Uninitialized() -> pointer {
    Assert(_storage);
    Assert(_size < _capacity);

    return ((T*)&_storage[_size++]);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
template <typename _Arg0, typename... _Args>
void TStack<T, _IsPod>::Push(_Arg0&& arg0, _Args&&... args) {
    Meta::Construct(Push_Uninitialized(), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
bool TStack<T, _IsPod>::Pop(pointer pvalue/* = nullptr */) {
    if (0 == _size)
        return false;

    Assert(_storage);

    T& elt = _storage[--_size];
    if (pvalue)
        *pvalue = std::move(elt);
    IF_CONSTEXPR(false == _IsPod)
        Meta::Destroy(&elt);

    return true;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
bool TStack<T, _IsPod>::Contains(const_reference item) const {
    return (end() != std::find(begin(), end(), item));
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
auto TStack<T, _IsPod>::Allocate(size_type count) -> pointer {
    if (_size + count > _capacity)
        return nullptr;

    Assert(_storage);

    pointer const p = &_storage[_size];
    _size += count;

    return p;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
bool TStack<T, _IsPod>::DeallocateIFP(pointer p, size_type count) {
    Assert(_size > 0);
    Assert(p >= _storage);
    Assert(p + count <= _storage + _size);

    // can only delete the last block allocated !
    if ((p + count) != (_storage + _size))
        return false;

    if (false == _IsPod) {
        for (size_t i = _size - count; i < _size; ++i)
            Meta::Destroy(&_storage[i]);
    }

    _size -= count;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void TStack<T, _IsPod>::Deallocate_AssertLIFO(pointer p, size_type count) {
    if (false == DeallocateIFP(p, count))
        AssertNotReached();
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void TStack<T, _IsPod>::clear() {
    IF_CONSTEXPR(false == _IsPod) {
        for (size_t i = 0; i < _size; ++i)
            Meta::Destroy(&_storage[i]);
    }
    _size = 0;
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void TStack<T, _IsPod>::Swap(TStack& other) {
    std::swap(_size, other._size);
    std::swap(_capacity, other._capacity);
    std::swap(_storage, other._storage);
}
//----------------------------------------------------------------------------
template <typename T, bool _IsPod>
void swap(TStack<T, _IsPod>& lhs, TStack<T, _IsPod>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment = std::alignment_of<T>::value >
class TFixedSizeStack : public TStack<T> {
public:
    typedef TStack<T> parent_type;

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

    TFixedSizeStack() : parent_type(reinterpret_cast<pointer>(&_insitu), _Capacity) {}

    TFixedSizeStack(TFixedSizeStack&& ) = delete;
    TFixedSizeStack& operator =(TFixedSizeStack&& rvalue) = delete;

    TFixedSizeStack(const TFixedSizeStack& other) = delete;
    TFixedSizeStack& operator =(const TFixedSizeStack& other) = delete;

    void Swap(TStack<T>& other) = delete;

    // very special behavior which sets capacity to 0, used by allocators
    void ForbidFurtherAccess() {
        Assert(0 == _size);
        _capacity = 0; // every next call to Push() will fail
        _storage = nullptr;
        Assert(parent_type::full());
    }

private:
    // /!\ won't call any ctor or dtor, values are considered as undefined
    typename ALIGNED_STORAGE(sizeof(T) * _Capacity, _Alignment) _insitu;

    using parent_type::_size;
    using parent_type::_capacity;
    using parent_type::_storage;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Less = Meta::TLess<T>, bool _IsPod = Meta::TIsPod<T>::value>
class TStackHeapAdapter : private _Less {
public:
    typedef TStack<T, _IsPod> stack_type;
    typedef typename stack_type::pointer pointer;
    typedef typename stack_type::const_pointer const_pointer;
    typedef typename stack_type::size_type size_type;

    explicit TStackHeapAdapter(const TMemoryView<T>& storage) : _stack(storage) {}
    TStackHeapAdapter(const TMemoryView<T>& storage, _Less&& pred) : _Less(std::move(pred)), _stack(storage) {}

    size_type capacity() const { return _stack.capacity(); }
    size_type size() const { return _stack.size(); }
    bool empty() const { return _stack.empty(); }

    const_pointer PeekHeap() const {
        return _stack.Peek();
    }

    template <typename _Arg0, typename... _Args>
    void PushHeap(_Arg0&& arg0, _Args&&... args) {
        _stack.Push(std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
        std::push_heap(_stack.begin(), _stack.end(), static_cast<_Less&>(*this));
    }

    bool PopHeap(pointer pvalue) {
        std::pop_heap(_stack.begin(), _stack.end(), static_cast<_Less&>(*this));
        return _stack.Pop(pvalue);
    }

    void clear() {
        _stack.clear();
    }

    inline friend void swap(TStackHeapAdapter& lhs, TStackHeapAdapter& rhs) {
        swap(static_cast<_Less&>(lhs), static_cast<_Less&>(rhs));
        lhs._stack.Swap(rhs._stack);
    }

private:
    stack_type _stack;
};
//----------------------------------------------------------------------------
template <typename T, typename _Less>
using TPODStackHeapAdapter = TStackHeapAdapter<T, _Less, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

template <typename T, bool _IsPod>
inline void* operator new(size_t sizeInBytes, PPE::TStack<T, _IsPod>& stack) {
    Assert(sizeInBytes == sizeof(T));
    return stack.Push_Uninitialized();
}
template <typename T, bool _IsPod>
inline void operator delete(void* ptr, PPE::TStack<T, _IsPod>& stack) {
    Assert_NoAssume(stack.AliasesToContainer(static_cast<T*>(ptr)));
    AssertNotImplemented(); // don't know the size of the block
}
