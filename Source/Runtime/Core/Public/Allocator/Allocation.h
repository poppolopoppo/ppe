#pragma once

#include "Core.h"

#include "Allocator/Alloca.h"
#include "Allocator/Mallocator.h"
#include "Allocator/StackAllocator.h"
#include "Allocator/TrackingAllocator.h"
#include "Memory/MemoryDomain.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_MEMORYDOMAINS
template <typename _Allocator, typename _Tag>
using TDecorateAllocator = TTrackingAllocator< _Tag, _Allocator >;
#else
template <typename _Allocator, typename _Tag>
using TDecorateAllocator = _Allocator;
#endif
//----------------------------------------------------------------------------
#define DECORATE_ALLOCATOR(_Domain, ...) \
    ::PPE::TDecorateAllocator< COMMA_PROTECT(__VA_ARGS__), MEMORYDOMAIN_TAG(_Domain) >
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define ALLOCATOR(_Domain, ...) \
    DECORATE_ALLOCATOR(_Domain, ::PPE::TMallocator<__VA_ARGS__>)
//----------------------------------------------------------------------------
#define ALIGNED_ALLOCATOR(_Domain, T, _Alignment) \
    DECORATE_ALLOCATOR(_Domain, ::PPE::TMallocator<COMMA_PROTECT(T) COMMA _Alignment>)
//----------------------------------------------------------------------------
#define STACK_ALLOCATOR(T) \
    ::PPE::TStackAllocator<T> // don't decorate TStackAllocator<> to avoid double logging with "Alloca" domain
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define OVERRIDE_CLASS_ALLOCATOR(_Allocator) \
public: \
    void* operator new(size_t ) { \
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
} //!namespace PPE