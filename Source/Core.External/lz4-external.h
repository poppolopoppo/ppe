#pragma once

#include "Core/Allocator/ThreadLocalHeap.h"
#include "Core/Allocator/TrackingMalloc.h"
#include "Core/Memory/MemoryDomain.h"

#ifndef EXPORT_CORE_EXTERNAL_LZ4
#   pragma warning(push)
#   pragma push_macro("ALLOCATOR")
#   pragma push_macro("FORCE_INLINE")
#   pragma push_macro("malloc")
#   pragma push_macro("free")
#endif

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

#ifdef _MSC_VER
#   pragma warning(disable: 6239) // (<non-zero constant> && <expression>) always evaluates to the result of <expression>.  Did you intend to use the bitwise-and operator?
#   if _HAS_CXX17
#       pragma warning(disable: 5033) // 'register' is no longer a supported storage class
#   endif
#endif

#ifndef EXPORT_CORE_EXTERNAL_LZ4

#   include "Core.External/lz4/lib/lz4.h"
#   include "Core.External/lz4/lib/lz4hc.h"

// clean the mess done by lz4-config.h :

#   pragma pop_macro("ALLOCATOR")
#   pragma pop_macro("FORCE_INLINE")
#   pragma pop_macro("malloc")
#   pragma pop_macro("free")

#   pragma warning(pop)

#endif //!EXPORT_CORE_EXTERNAL_LZ4
