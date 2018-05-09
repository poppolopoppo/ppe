#pragma once

#include "Core/Allocator/TrackingMalloc.h"
#include "Core/Memory/MemoryDomain.h"

#ifndef EXPORT_CORE_EXTERNAL_LZ4
PRAGMA_MSVC_WARNING_PUSH()
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

#define malloc(sz) TRACKING_MALLOC(LZ4, sz)
#define free(p) Core::tracking_free(p)

PRAGMA_MSVC_WARNING_DISABLE(5033) // 'register' is no longer a supported storage class (C++17)
PRAGMA_MSVC_WARNING_DISABLE(6239) // (<non-zero constant> && <expression>) always evaluates to the result of <expression>.  Did you intend to use the bitwise-and operator?

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_CORE_EXTERNAL_LZ4

#   include "Core.External/lz4/lib/lz4.h"
#   include "Core.External/lz4/lib/lz4hc.h"

// clean the mess done by lz4-config.h :

#   pragma pop_macro("ALLOCATOR")
#   pragma pop_macro("FORCE_INLINE")
#   pragma pop_macro("malloc")
#   pragma pop_macro("free")

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_CORE_EXTERNAL_LZ4
