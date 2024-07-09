#pragma once

#include "Core.h"

#ifndef EXPORT_PPE_EXTERNAL_SPIRV_CROSS
    PRAGMA_MSVC_WARNING_PUSH()
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#endif

PRAGMA_MSVC_WARNING_DISABLE(4065) // switch statement contains 'default' but no 'case' labels
PRAGMA_MSVC_WARNING_DISABLE(4389) // '!=': signed/unsigned mismatch
PRAGMA_MSVC_WARNING_DISABLE(4668) // undefined preprocessor macro, replacing with '0'
PRAGMA_MSVC_WARNING_DISABLE(4701) // potentially uninitialized local variable
PRAGMA_MSVC_WARNING_DISABLE(4702) // unreachable code
PRAGMA_MSVC_WARNING_DISABLE(4703) // potentially uninitialized local pointer variable
PRAGMA_MSVC_WARNING_DISABLE(4706) // assignment within conditional expression
PRAGMA_MSVC_WARNING_DISABLE(4800) // implicit conversion from unsigned __int64 to bool, possible loss of information
PRAGMA_MSVC_WARNING_DISABLE(4805) // '!=': unsafe mix of type 'const uint32_t' and type 'const bool' in operation
PRAGMA_MSVC_WARNING_DISABLE(4996) // 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

#ifdef __clang__
#   pragma clang system_header
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wdeprecated-this-capture"
#endif

#ifdef __gcc__
#   pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_SPIRV_CROSS

#   pragma include_alias("spirv.h", "External/spirv-cross/SPIRV-Cross.git/spirv.h")
#   pragma include_alias("spirv.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv.hpp")
#   pragma include_alias("spirv_cfg.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_cfg.hpp")
#   pragma include_alias("spirv_common.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_common.hpp")
#   pragma include_alias("spirv_cpp.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_cpp.hpp")
#   pragma include_alias("spirv_cross.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_cross.hpp")
#   pragma include_alias("spirv_cross_c.h", "External/spirv-cross/SPIRV-Cross.git/spirv_cross_c.h")
#   pragma include_alias("spirv_cross_containers.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_cross_containers.hpp")
#   pragma include_alias("spirv_cross_error_handling.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_cross_error_handling.hpp")
#   pragma include_alias("spirv_cross_parsed_ir.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_cross_parsed_ir.hpp")
#   pragma include_alias("spirv_cross_util.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_cross_util.hpp")
#   pragma include_alias("spirv_glsl.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_glsl.hpp")
#   pragma include_alias("spirv_hlsl.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_hlsl.hpp")
#   pragma include_alias("spirv_msl.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_msl.hpp")
#   pragma include_alias("spirv_parser.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_parser.hpp")
#   pragma include_alias("spirv_reflect.hpp", "External/spirv-cross/SPIRV-Cross.git/spirv_reflect.hpp")

#   include "External/spirv-cross/SPIRV-Cross.git/spirv_cross.hpp"

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_SPIRV_CROSS
