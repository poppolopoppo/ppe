#pragma once

#include "Core.h"

#ifndef EXPORT_PPE_EXTERNAL_SPIRV_TOOLS
    PRAGMA_MSVC_WARNING_PUSH()
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#endif

PRAGMA_MSVC_WARNING_DISABLE(4127) // conditional expression is constant
PRAGMA_MSVC_WARNING_DISABLE(4389) // signed/unsigned mismatch
PRAGMA_MSVC_WARNING_DISABLE(4702) // unreachable code
PRAGMA_MSVC_WARNING_DISABLE(4706) // assignment within conditional expression
PRAGMA_MSVC_WARNING_DISABLE(4996) // this function or variable may be unsafe

#ifdef __clang__
#   pragma clang system_header
//#   pragma clang diagnostic ignored "-Wunused-function"
#endif

#ifdef __gcc__
#   pragma GCC system_header
#endif

#include "spirv-tools-headers.generated.h"

#ifndef EXPORT_PPE_EXTERNAL_SPIRV_TOOLS

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_SPIRV_TOOLS
