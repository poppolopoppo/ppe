#pragma once

#include "HAL/PlatformIncludes.h"

#ifndef EXPORT_PPE_EXTERNAL_DETOURS
    PRAGMA_MSVC_WARNING_PUSH()
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif

#else
#   define DETOURS_INTERNAL
#endif

// PRAGMA_MSVC_WARNING_DISABLE(4244) //  '+=': conversion from 'UINT' to 'UINT8', possible loss of data
// PRAGMA_MSVC_WARNING_DISABLE(4310) //  cast truncates constant value
// PRAGMA_MSVC_WARNING_DISABLE(4701) //  assignment within conditional expression
// PRAGMA_MSVC_WARNING_DISABLE(4706) //  potentially uninitialized local variable 'XXX' used

#ifdef __clang__
#    pragma clang system_header
#endif

#ifdef __clang__
#   pragma GCC system_headerp
#   pragma clang diagnostic ignored "-Wreorder-ctor"
#   pragma clang diagnostic ignored "-Wsign-compare"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include "External/detours/detours.git/src/detver.h"
#include "External/detours/detours.git/src/detours.h"

#ifndef EXPORT_PPE_EXTERNAL_DETOURS
#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()
#endif
