#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformIncludes.h"

#include "Allocator/Allocation.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/ConstChar.h"

# define VMA_IMPLEMENTATION                 (1)
# define VMA_STATIC_VULKAN_FUNCTIONS        (0)
# define VMA_DYNAMIC_VULKAN_FUNCTIONS       (1)

# define VMA_RECORDING_ENABLED              (0)
# define VMA_STATS_STRING_ENABLED           (!(USE_PPE_FINAL_RELEASE||USE_PPE_PROFILING))

# define VMA_DEDICATED_ALLOCATION           (1)
# define VMA_BIND_MEMORY2                   (1)
# define VMA_MEMORY_BUDGET                  (1)

# define VMA_USE_STL_SHARED_MUTEX           (1)

#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) \
    TRACKING_ALIGNED_MALLOC(VMA, size, alignment)
#define VMA_SYSTEM_ALIGNED_FREE(ptr) \
    TRACKING_FREE(VMA, ptr)

# define VMA_DEBUG_ALWAYS_DEDICATED_MEMORY  (0)
# define VMA_DEBUG_INITIALIZE_ALLOCATIONS   (USE_PPE_MEMORY_DEBUGGING)
# define VMA_DEBUG_DETECT_CORRUPTION        (USE_PPE_MEMORY_DEBUGGING)
# define VMA_DEBUG_GLOBAL_MUTEX             (0) // will be externally synchronized

# define VMA_ASSERT(expr)                   Assert_NoAssume( expr )
#if USE_PPE_MEMORY_DEBUGGING
# define VMA_HEAVY_ASSERT(expr)             Assert_NoAssume( expr )
#else
# define VMA_HEAVY_ASSERT(expr)             NOOP( expr )
#endif

#if USE_PPE_LOGGER
LOG_CATEGORY(, VMA)
# if PPE_VA_OPT_SUPPORTED
#   define VMA_DEBUG_LOG_FORMAT(format, ...) PPE_LOG_PRINTF(VMA, Debug, format __VA_OPT__(,) __VA_ARGS__)
# else
#   define VMA_DEBUG_LOG_FORMAT(format, ...) PPE_LOG_PRINTF(VMA, Debug, format ,## __VA_ARGS__)
# endif
#else
# define VMA_DEBUG_LOG_FORMAT(format, ...)
#endif

template<typename T>
struct VmaStlAllocator; // forward declare from VMA

#define _VMA_VECTOR // override default vector class of VMA (better memory usage and tracking)
/* Class with interface compatible with subset of std::vector.
T must be POD because constructors and destructors are not called and memcpy is
used for these objects. */
template<typename T>
using TPPEVectorForVma = VECTOR(VMA, PPE::Meta::TEnableIf<PPE::Meta::is_pod_v<T> COMMA T>);
template<typename T, typename AllocatorT>
class VmaVector : public TPPEVectorForVma<T> {
public:
    using typename TPPEVectorForVma<T>::value_type;
    using typename TPPEVectorForVma<T>::iterator;
    using typename TPPEVectorForVma<T>::const_iterator;
    using TPPEVectorForVma<T>::TPPEVectorForVma;
    using TPPEVectorForVma<T>::operator =;
    using TPPEVectorForVma<T>::operator [];

    CONSTEXPR VmaVector(VmaStlAllocator<T>) NOEXCEPT {}
    VmaVector(std::size_t count, VmaStlAllocator<T>) {
        TPPEVectorForVma<T>::resize_Uninitialized(count);
    }
};
template<typename T, typename allocatorT>
static void VmaVectorInsert(VmaVector<T, allocatorT>& vec, std::size_t index, const T& item) {
    vec.insert(index, item);
}
template<typename T, typename allocatorT>
static void VmaVectorRemove(VmaVector<T, allocatorT>& vec, std::size_t index) {
    vec.erase(vec.begin() + index);
}

#define _VMA_SMALL_VECTOR // override default small vector class of VMA (better memory usage and tracking)
/*
This is a vector (a variable-sized array), optimized for the case when the array is small.

It contains some number of elements in-place, which allows it to avoid heap allocation
when the actual number of elements is below that threshold. This allows normal "small"
cases to be fast without losing generality for large inputs.
*/
template<typename T, std::size_t N>
using TPPESmallVectorForVma = VECTORINSITU(VMA, PPE::Meta::TEnableIf<PPE::Meta::is_pod_v<T> COMMA T>, N);
template<typename T, typename AllocatorT, std::size_t N>
class VmaSmallVector : public TPPESmallVectorForVma<T, N> {
public:
    using typename TPPESmallVectorForVma<T, N>::value_type;
    using typename TPPESmallVectorForVma<T, N>::iterator;
    using typename TPPESmallVectorForVma<T, N>::const_iterator;
    using TPPESmallVectorForVma<T, N>::TPPESmallVectorForVma;
    using TPPESmallVectorForVma<T, N>::operator =;
    using TPPESmallVectorForVma<T, N>::operator [];

    CONSTEXPR VmaSmallVector(VmaStlAllocator<T>) NOEXCEPT {}
    VmaSmallVector(std::size_t count, VmaStlAllocator<T>) {
        TPPESmallVectorForVma<T, N>::resize_Uninitialized(count);
    }
};

#ifdef __gcc__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#   pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#ifdef _MSC_VER
#   pragma warning (push, 0)
#   pragma warning (disable: 4100)
#   pragma warning (disable: 4296)
#   pragma warning (disable: 4062)
#   pragma warning (disable: 4701)
#   pragma warning (disable: 4703)
#   pragma warning (disable: 4826)
#endif
#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wignored-qualifiers"
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#   pragma clang diagnostic ignored "-Wnullability-completeness"
#   pragma clang diagnostic ignored "-Wswitch"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wunused-function"
#endif

#include "External/vma/vma.git/include/vk_mem_alloc.h"

#ifdef __gcc__
#   pragma GCC diagnostic pop
#endif
#ifdef _MSC_VER
#   pragma warning (pop)
#endif
#ifdef __clang__
#   pragma clang diagnostic pop
#endif
