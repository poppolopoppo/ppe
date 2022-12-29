#pragma once

#include "Core_fwd.h"

#ifndef EXPORT_PPE_EXTERNAL_MINHOOK
    PRAGMA_MSVC_WARNING_PUSH()
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#endif

PRAGMA_MSVC_WARNING_DISABLE(4244) //  '+=': conversion from 'UINT' to 'UINT8', possible loss of data
PRAGMA_MSVC_WARNING_DISABLE(4310) //  cast truncates constant value
PRAGMA_MSVC_WARNING_DISABLE(4701) //  assignment within conditional expression
PRAGMA_MSVC_WARNING_DISABLE(4706) //  potentially uninitialized local variable 'XXX' used

#ifdef __clang__
#    pragma clang system_header
#endif

#ifdef __clang__
#   pragma GCC system_header
#   pragma clang diagnostic ignored "-Wunused-parameter"
#endif

#include "HAL/PlatformIncludes.h"

#ifndef EXPORT_PPE_EXTERNAL_MINHOOK
#   include "External/minhook/minhook.git/include/MinHook.h"

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()
#endif
