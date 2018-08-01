#pragma once

// mark files including as system headers : warnings will be ignored starting from here

#ifdef CPP_CLANG
#   pragma clang system_header
#endif
#ifdef CPP_GCC
#   pragma GCC system_header
#endif
#ifdef CPP_VISUALSTUDIO
#   pragma system_header
#endif

#include "HAL/TargetPlatform.h"
#include PPE_HAL_MAKEINCLUDE(PlatformIncludes)
