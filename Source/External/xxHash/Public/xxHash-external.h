#pragma once

#include "HAL/PlatformMacros.h"

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_XXHASH
    PRAGMA_MSVC_WARNING_PUSH()
#   pragma push_macro("FORCE_INLINE")
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#endif

#define USE_PPE_XXH3 1 // new algorithm, 2x to 3x times faster

#define XXH_INLINE_ALL // inlining for better performance
#define XXH_STATIC_LINKING_ONLY // unlocks XXH3

// overrides xxhash detection for compiler intrinsics
#if defined(__AVX2__)
#    define XXH_VECTOR XXH_AVX2
#else
#    define XXH_VECTOR XXH_SSE2
#endif

PRAGMA_MSVC_WARNING_DISABLE(4244) // 'argument': conversion from 'uint64_t' to 'uint32_t', possible loss of data

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wpass-failed=transform-warning"
#endif

#ifndef EXPORT_PPE_EXTERNAL_XXHASH

#   include "External/xxHash/xxHash.git/xxhash.h"

// clean the mess done by xxHash-config.h :

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

#   pragma pop_macro("FORCE_INLINE")

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_XXHASH
