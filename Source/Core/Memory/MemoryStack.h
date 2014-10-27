#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MemoryStack : public MemoryView<T> {
public:
    typedef MemoryView<T> base_type;

    using typename base_type::value_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::reference;
    using typename base_type::const_reference;

    using typename base_type::size_type;
    using typename base_type::difference_type;

    using typename base_type::iterator;
    using typename base_type::iterator_category;

    MemoryStack();
    explicit MemoryStack(const MemoryView<T>& other);
    MemoryStack(pointer storage, size_type capacity);
    ~MemoryStack();

    MemoryStack(const MemoryStack& other);
    MemoryStack& operator =(const MemoryStack& other);

    size_type capacity() const { return _capacity; }
    void resize(size_type size) { Assert(size <= _capacity); _size = size; }

    // /!\ won't call any ctor or dtor, values are considered as undefined

    pointer Allocate(size_type count);
    void Deallocate(pointer p, size_type count);
    void Deallocate_AssertLIFO(pointer p, size_type count);

    pointer Push(size_type count) { return Allocate(count); }
    void Pop(pointer p, size_type count) { Deallocate_AssertLIFO(p, count); }

    void Clear();

    void PushPOD(const_reference pod);
    bool PopPOD(pointer p);

    void Swap(MemoryStack& other);

protected:
    size_type _capacity;
};
//----------------------------------------------------------------------------
template <typename T>
MemoryStack<T>::MemoryStack()
:   base_type(nullptr, 0), _capacity(0) {}
//----------------------------------------------------------------------------
template <typename T>
MemoryStack<T>::MemoryStack(const MemoryView<T>& other)
:   base_type(other.Pointer(), 0), _capacity(other.size()) {}
//----------------------------------------------------------------------------
template <typename T>
MemoryStack<T>::MemoryStack(pointer storage, size_type capacity)
:   base_type(storage, 0), _capacity(capacity) {
    Assert(storage || 0 == capacity);
}
//----------------------------------------------------------------------------
template <typename T>
MemoryStack<T>::~MemoryStack() {}
//----------------------------------------------------------------------------
template <typename T>
MemoryStack<T>::MemoryStack(const MemoryStack& other)
:   base_type(other), _capacity(other._capacity) {}
//----------------------------------------------------------------------------
template <typename T>
MemoryStack<T>& MemoryStack<T>::operator =(const MemoryStack& other) {
    base_type::operator =(other);
    _capacity = other._capacity;
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T>
auto MemoryStack<T>::Allocate(size_type count) -> pointer {
    if (base_type::_size + count > _capacity)
        return nullptr;
    pointer const p = &base_type::_storage[base_type::_size];
    base_type::_size += count;
    return p;
}
//----------------------------------------------------------------------------
template <typename T>
void MemoryStack<T>::Deallocate(pointer p, size_type count) {
    Assert(count <= base_type::_size);
    Assert(p + count <= base_type::end());
    // Try to free the memory, will only succeed if alloc/dealloc are in LIFO order
    if (p + count == base_type::end())
        base_type::_size -= count;
}
//----------------------------------------------------------------------------
template <typename T>
void MemoryStack<T>::Deallocate_AssertLIFO(pointer p, size_type count) {
    Assert(p + count == base_type::end()); // ensures LIFO order
    return Deallocate(p, count);
}
//----------------------------------------------------------------------------
template <typename T>
void MemoryStack<T>::Clear() {
    base_type::_size = 0;
}
//----------------------------------------------------------------------------
template <typename T>
void MemoryStack<T>::PushPOD(const_reference pod) {
    pointer const p = Allocate(1);
    Assert(p);
    *p = std::move(pod);
}
//----------------------------------------------------------------------------
template <typename T>
bool MemoryStack<T>::PopPOD(pointer p) {
    if (0 == base_type::_size)
        return false;
    *p = std::move(_storage[--base_type::_size]);
    return true;
}
//----------------------------------------------------------------------------
template <typename T>
void MemoryStack<T>::Swap(MemoryStack& other) {
    base_type::Swap(other);
    std::swap(other._capacity, _capacity);
}
//----------------------------------------------------------------------------
template <typename T>
void swap(MemoryStack<T>& lhs, MemoryStack<T>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment = std::alignment_of<T>::value >
class StaticStack : public MemoryStack<T> {
public:
    typedef MemoryStack<T> base_type;

    using typename base_type::value_type;
    using typename base_type::pointer;
    using typename base_type::const_pointer;
    using typename base_type::reference;
    using typename base_type::const_reference;

    using typename base_type::size_type;
    using typename base_type::difference_type;

    using typename base_type::iterator;
    using typename base_type::iterator_category;

    StaticStack();
    ~StaticStack();

    StaticStack(StaticStack&& rvalue);
    StaticStack& operator =(StaticStack&& rvalue);

    StaticStack(const StaticStack& other) = delete;
    StaticStack& operator =(const StaticStack& other) = delete;

    pointer Pointer() { return reinterpret_cast<pointer>(&_static); }
    const_pointer Pointer() const { return reinterpret_cast<const_pointer>(&_static); }

    void Swap(MemoryStack<T>& other) = delete;

protected:
    // /!\ won't call any ctor or dtor, values are considered as undefined
    typename ALIGNED_STORAGE(sizeof(T) * _Capacity, _Alignment) _static;
};
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment>
StaticStack<T, _Capacity, _Alignment>::StaticStack()
: base_type(Pointer(), _Capacity) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment>
StaticStack<T, _Capacity, _Alignment>::~StaticStack() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment>
StaticStack<T, _Capacity, _Alignment>::StaticStack(StaticStack&&)
: base_type(Pointer(), _Capacity) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Capacity, size_t _Alignment>
auto StaticStack<T, _Capacity, _Alignment>::operator =(StaticStack&&) -> StaticStack& {
    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
