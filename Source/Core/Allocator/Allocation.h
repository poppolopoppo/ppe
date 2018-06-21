#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Alloca.h"
#include "Core/Allocator/Mallocator.h"
#include "Core/Allocator/StackAllocator.h"
#include "Core/Allocator/TrackingAllocator.h"
#include "Core/Memory/MemoryDomain.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_CORE_MEMORYDOMAINS
template <typename _Allocator, typename _Tag>
using TDecorateAllocator = TTrackingAllocator< _Tag, _Allocator >;
#else
template <typename _Allocator, typename _Tag>
using TDecorateAllocator = _Allocator;
#endif
//----------------------------------------------------------------------------
#define DECORATE_ALLOCATOR(_Domain, ...) \
    ::Core::TDecorateAllocator< COMMA_PROTECT(__VA_ARGS__), MEMORYDOMAIN_TAG(_Domain) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALLOCATOR(_Domain, ...) \
    DECORATE_ALLOCATOR(_Domain, ::Core::TMallocator<__VA_ARGS__>)
//----------------------------------------------------------------------------
#define ALIGNED_ALLOCATOR(_Domain, T, _Alignment) \
    DECORATE_ALLOCATOR(_Domain, ::Core::TMallocator<COMMA_PROTECT(T) COMMA _Alignment>)
//----------------------------------------------------------------------------
#define STACK_ALLOCATOR(T) \
    ::Core::TStackAllocator<T> // don't decorate TStackAllocator<> to avoid double logging with "Alloca" domain
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define OVERRIDE_CLASS_ALLOCATOR(_Allocator) \
public: \
    void* operator new(size_t size) { \
        return _Allocator().allocate(1); \
    } \
    void* operator new(size_t, void* ptr) { \
        Assert(ptr); \
        return ptr; \
    } \
    \
    void operator delete(void* ptr) { \
        using pointer = typename std::allocator_traits<_Allocator>::pointer; \
        _Allocator().deallocate(reinterpret_cast<pointer>(ptr), 1); \
    } \
    void operator delete(void* ptr, size_t) { operator delete(ptr); } \
    void operator delete(void*, void*) {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
