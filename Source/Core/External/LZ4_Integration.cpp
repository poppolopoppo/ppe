#include "stdafx.h"

#include "Core/Allocator/TrackingMalloc.h"

// this file is isolated so it's not a problem to do this :
#undef ALLOCATOR
#undef FORCE_INLINE

#ifdef malloc
#   undef malloc
#endif
#ifdef free
#   undef free
#endif

#define malloc(sz) Core::tracking_malloc_thread_local<MEMORY_DOMAIN_TAG(LZ4)>(sz)
#define free(p) Core::tracking_free_thread_local<MEMORY_DOMAIN_TAG(LZ4)>(p)

#ifdef PLATFORM_WINDOWS
#   pragma warning(push)
#   pragma warning(disable: 6239) // (<non-zero constant> && <expression>) always evaluates to the result of <expression>.  Did you intend to use the bitwise-and operator?
#endif

#include "External/lz4.c"

#undef malloc
#undef free

#ifdef PLATFORM_WINDOWS
#   pragma warning(pop)
#endif
