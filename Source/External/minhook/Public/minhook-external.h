#pragma once

#include "Core_fwd.h"

#ifndef EXPORT_PPE_EXTERNAL_MINHOOK
    PRAGMA_MSVC_WARNING_PUSH()
#endif

PRAGMA_MSVC_WARNING_DISABLE(4244) //  '+=': conversion from 'UINT' to 'UINT8', possible loss of data
PRAGMA_MSVC_WARNING_DISABLE(4310) //  cast truncates constant value
PRAGMA_MSVC_WARNING_DISABLE(4701) //  assignment within conditional expression
PRAGMA_MSVC_WARNING_DISABLE(4706) //  potentially uninitialized local variable 'XXX' used

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_MINHOOK
#   include "External/minhook/minhook.git/include/MinHook.h"

    PRAGMA_MSVC_WARNING_POP()
#endif
