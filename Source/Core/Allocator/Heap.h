#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"
#include "Core/Meta/Singleton.h"

#ifdef _DEBUG
#   define USE_HEAP_VALIDATION
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FHeap {
public:
    struct current_process_t{};

    FHeap();
    explicit FHeap(current_process_t);
    FHeap(FHeap&& rvalue);
    ~FHeap();

    FHeap(const FHeap&) = delete;
    FHeap& operator =(const FHeap&) = delete;

    void*   Handle() const { return _handle; }

    void*   Malloc(size_t size);
    void    Free(void *ptr);
    void*   Calloc(size_t nmemb, size_t size);
    void*   Realloc(void *ptr, size_t size);

    void*   AlignedMalloc(size_t size, size_t alignment);
    void    AlignedFree(void *ptr);
    void*   AlignedCalloc(size_t nmemb, size_t size, size_t alignment);
    void*   AlignedRealloc(void *ptr, size_t size, size_t alignment);

    template <size_t _Alignment>
    void*   Malloc(size_t size, typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void    Free(void *ptr, typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   Calloc(size_t nmemb, size_t size, typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   Realloc(void *ptr, size_t size, typename std::enable_if< TIsNaturalyAligned<_Alignment>::value >::type* = 0);

    template <size_t _Alignment>
    void*   Malloc(size_t size, typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void    Free(void *ptr, typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   Calloc(size_t nmemb, size_t size, typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0);
    template <size_t _Alignment>
    void*   Realloc(void *ptr, size_t size, typename std::enable_if< !TIsNaturalyAligned<_Alignment>::value >::type* = 0);

    void    Swap(FHeap& other);

private:
    void*   _handle;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Tag>
class THeapSingleton : Meta::TSingleton<FHeap, _Tag> {
    typedef Meta::TSingleton<FHeap, _Tag> parent_type;
public:
    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;
    using parent_type::Create;
};
//----------------------------------------------------------------------------
#define HEAP_SINGLETON_DEF(_NAME) namespace Heaps { \
    struct _NAME : public Core::THeapSingleton<_NAME> {}; \
    }
//----------------------------------------------------------------------------
HEAP_SINGLETON_DEF(Process);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Allocator/Heap-inl.h"
