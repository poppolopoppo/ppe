#pragma once

#include "Core.h"

#ifndef EXPORT_PPE_EXTERNAL_GLSLANG
    PRAGMA_MSVC_WARNING_PUSH()
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#endif

// PRAGMA_MSVC_WARNING_DISABLE(4065) // switch statement contains 'default' but no 'case' labels

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#define ENABLE_OPT 1

#include "glslang-headers.generated.h"

#ifndef EXPORT_PPE_EXTERNAL_GLSLANG

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_GLSLANG
