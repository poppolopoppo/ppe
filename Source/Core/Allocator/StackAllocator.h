#pragma once

#include "Core/Core.h"

#include "Core/Allocator/AllocatorBase.h"
#include "Core/Memory/MemoryStack.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef MemoryStack<uint8_t> StackAllocatorMemory;
//----------------------------------------------------------------------------
#define MALLOCA_STACKALLOCATORMEMORY(_SIZE) \
    MALLOCA_STACK(uint8_t, _SIZE)
//----------------------------------------------------------------------------
template <typename T>
class StackAllocator : public AllocatorBase<T> {
public:
    typedef AllocatorBase<T> base_type;

    typedef typename base_type::pointer pointer;
    typedef typename base_type::size_type size_type;

    template<typename U>
    struct rebind
    {
        typedef StackAllocator<U> other;
    };

    StackAllocator() throw()
        : _memoryStack(nullptr) {}
    StackAllocator(StackAllocatorMemory& memoryStack) throw()
        : _memoryStack(&memoryStack) {}
    StackAllocator(const StackAllocator& other) throw()
        : _memoryStack(other.MemoryStack()) {}
    template <typename U>
    StackAllocator(const StackAllocator<U>& other) throw()
        : _memoryStack(other.MemoryStack()) {}

    StackAllocator& operator=(const StackAllocator&);
    template <typename U>
    StackAllocator& operator=(const StackAllocator<U>&);

    size_type max_size() const;

    pointer allocate(size_type n);
    pointer allocate(size_type n, const void* /*hint*/) { return allocate(n); }
    void deallocate(void* p, size_type n);

    StackAllocatorMemory* MemoryStack() const { return _memoryStack; }

private:
    StackAllocatorMemory* _memoryStack;
};
//----------------------------------------------------------------------------
template <typename T>
auto StackAllocator<T>::operator=(const StackAllocator&) -> StackAllocator& {
    _memoryStack = other.MemoryStack();
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
template<typename U>
auto StackAllocator<T>::operator =(const StackAllocator<U>& other) -> StackAllocator& {
    _memoryStack = other.MemoryStack();
    return *this;
}
//----------------------------------------------------------------------------
template <typename T>
auto StackAllocator<T>::max_size() const -> size_type {
    enum { Alignment = std::alignment_of<T>::value };

    if (!_memoryStack)
        return 0;
    Assert(_memoryStack->capacity() >= _memoryStack->size());

    const size_t maxSizeInBytes = _memoryStack->capacity() - _memoryStack->size();
    if (maxSizeInBytes < Alignment)
        return 0;

    return (maxSizeInBytes - Alignment) / sizeof(T);
}
//----------------------------------------------------------------------------
template <typename T>
auto StackAllocator<T>::allocate(size_type n) -> pointer {
    enum { Alignment = std::alignment_of<T>::value };

    // The return value of allocate(0) is unspecified.
    // Mallocator returns NULL in order to avoid depending
    // on malloc(0)'s implementation-defined behavior
    // (the implementation can define malloc(0) to return NULL,
    // in which case the bad_alloc check below would fire).
    // All allocators can return nullptr in this case.
    if (n == 0)
        return nullptr;

    // All allocators should contain an integer overflow check.
    // The Standardization Committee recommends that std::length_error
    // be thrown in the case of integer overflow.
    if (n > max_size())
        throw std::length_error("StackAllocator<T>::allocate() - Integer overflow.");

    Assert(_memoryStack);
    uint8_t* const pv = _memoryStack->Allocate(n * sizeof(value_type));

    // Allocators should throw std::bad_alloc in the case of memory allocation failure.
    if (pv == nullptr)
        throw std::bad_alloc();

    Assert(IS_ALIGNED(Alignment, pv));
    return reinterpret_cast<pointer>(pv);
}
//----------------------------------------------------------------------------
template <typename T>
void StackAllocator<T>::deallocate(void* p, size_type n) {
    enum { Alignment = std::alignment_of<T>::value };

    Assert(p);
    Assert(IS_ALIGNED(Alignment, p));
    Assert(_memoryStack);

    _memoryStack->Deallocate(reinterpret_cast<typename StackAllocatorMemory::pointer>(p), n);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
bool operator ==(const StackAllocator<T>& lhs, const StackAllocator<T>& rhs) {
    return (lhs._memoryStack == rhs._memoryStack);
}
//----------------------------------------------------------------------------
template <typename T>
bool operator !=(const StackAllocator<T>& lhs, const StackAllocator<T>& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
template <typename T, typename _Other >
bool operator ==(const StackAllocator<T>& lhs, const _Other& rhs) {
    return false;
}
//----------------------------------------------------------------------------
template <typename T, typename _Other >
bool operator !=(const StackAllocator<T>& lhs, const _Other& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
