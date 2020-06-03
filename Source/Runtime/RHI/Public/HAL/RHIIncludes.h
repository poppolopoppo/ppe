#pragma once

// mark files including as system headers : warnings will be ignored starting from here

#ifdef CPP_CLANG
#   pragma clang system_header
#endif
#ifdef CPP_GCC
#   pragma GCC system_header
#endif
#if defined(CPP_VISUALSTUDIO) && defined(USE_PPE_MSVC_PRAGMA_SYSTEMHEADER)
#   pragma system_header
#endif

#include "HAL/TargetRHI.h"
#include PPE_RHI_MAKEINCLUDE(RHIIncludes)
