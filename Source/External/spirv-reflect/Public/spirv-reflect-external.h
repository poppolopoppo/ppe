#pragma once

#include "Core.h"

#ifndef EXPORT_PPE_EXTERNAL_SPIRV_REFLECT
    PRAGMA_MSVC_WARNING_PUSH()
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#endif

#ifdef CPP_CLANG
#   pragma clang system_header
#   pragma clang diagnostic ignored "-Wmissing-field-initializers"
#   pragma clang diagnostic ignored "-Wparentheses-equality"
#   pragma clang diagnostic ignored "-Wsign-compare"
#endif

#ifdef CPP_GCC
#   pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_SPIRV_REFLECT

#   include "External/spirv-reflect/SPIRV-Reflect.git/spirv_reflect.h"

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_SPIRV_REFLECT
