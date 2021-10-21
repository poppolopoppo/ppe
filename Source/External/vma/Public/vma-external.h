#pragma once

#include "Core_fwd.h"

#include "Diagnostic/Logger.h"
#include "IO/ConstChar.h"

# define VMA_IMPLEMENTATION                 1
# define VMA_STATIC_VULKAN_FUNCTIONS        0
# define VMA_DYNAMIC_VULKAN_FUNCTIONS       1

# define VMA_RECORDING_ENABLED              0
# define VMA_STATS_STRING_ENABLED           (!USE_PPE_FINAL_RELEASE)

# define VMA_DEDICATED_ALLOCATION           1
# define VMA_BIND_MEMORY2                   1
# define VMA_MEMORY_BUDGET                  1

# define VMA_USE_STL_CONTAINERS             1
# define VMA_USE_STL_VECTOR                 1
# define VMA_USE_STL_UNORDERED_MAP          1
# define VMA_USE_STL_LIST                   1
# define VMA_USE_STL_SHARED_MUTEX           1

# define VMA_DEBUG_ALWAYS_DEDICATED_MEMORY  0
# define VMA_DEBUG_INITIALIZE_ALLOCATIONS   (USE_PPE_DEBUG)
# define VMA_DEBUG_DETECT_CORRUPTION        (USE_PPE_MEMORY_DEBUGGING)
# define VMA_DEBUG_GLOBAL_MUTEX             0 // will be externally synchronized

# define VMA_ASSERT(expr)                   Assert_NoAssume( expr )
#if USE_PPE_MEMORY_DEBUGGING
# define VMA_HEAVY_ASSERT(expr)             Assert_NoAssume( expr )
#else
# define VMA_HEAVY_ASSERT(expr)             NOOP( expr )
#endif

# define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) \
    TRACKING_ALIGNED_MALLOC(VMA, size, alignment)
# define VMA_SYSTEM_ALIGNED_FREE(ptr) \
    TRACKING_FREE(VMA, ptr)

#if USE_PPE_LOGGER
LOG_CATEGORY(, VMA)
# define VMA_DEBUG_LOG(format, ...) LOG_PRINTF(VMA, Debug, WIDESTRING(format), #__VA_ARGS__)
#else
# define VMA_DEBUG_LOG(format, ...)
#endif

#ifdef CPP_GCC
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#   pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#ifdef CPP_VISUALSTUDIO
#   pragma warning (push, 0)
#   pragma warning (disable: 4100)
#   pragma warning (disable: 4296)
#   pragma warning (disable: 4701)
#   pragma warning (disable: 4703)
#endif
#ifdef CPP_CLANG
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#   pragma clang diagnostic ignored "-Wcast-align"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wnullability-completeness"
#endif

#include "External/vma/vma.git/include/vk_mem_alloc.h"

#ifdef CPP_GCC
#   pragma GCC diagnostic pop
#endif
#ifdef CPP_VISUALSTUDIO
#   pragma warning (pop)
#endif
#ifdef CPP_CLANG
#   pragma clang diagnostic pop
#endif
