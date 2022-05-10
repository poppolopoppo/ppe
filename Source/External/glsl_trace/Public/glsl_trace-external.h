#pragma once

#include "Core.h"

#ifndef EXPORT_PPE_EXTERNAL_GLSL_TRACE
    PRAGMA_MSVC_WARNING_PUSH()
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#endif

PRAGMA_MSVC_WARNING_DISABLE(4061) // enumerator 'XXX' in switch of enum 'YYY' is not explicitly handled by a case label
PRAGMA_MSVC_WARNING_DISABLE(4062) // enumerator 'XXX' in switch of enum 'YYY' is not handled
PRAGMA_MSVC_WARNING_DISABLE(4063) // case 'XXX' is not a valid value for switch of enum 'YYY'
PRAGMA_MSVC_WARNING_DISABLE(4100) // 'XXX': unreferenced formal parameter
PRAGMA_MSVC_WARNING_DISABLE(4189) // local variable is initialized but not referenced
PRAGMA_MSVC_WARNING_DISABLE(4244) // 'XXX': conversion from 'YYY' to 'ZZZ', possible loss of data

#ifdef CPP_CLANG
#   pragma clang system_header
#   pragma clang diagnostic ignored "-Wunused-function"
#   pragma clang diagnostic ignored "-Wunused-parameter"
#   pragma clang diagnostic ignored "-Wunused-variable"
#   pragma clang diagnostic ignored "-Wswitch"
#   pragma clang diagnostic ignored "-Wtautological-constant-out-of-range-compare"
#endif

#ifdef CPP_GCC
#   pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_GLSL_TRACE

#   include "External/glsl_trace/glsl_trace.git/shader_trace/include/ShaderTrace.h"

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#else

#	define ASSERT( _expr_ ) AssertRelease_NoAssume(_expr_)
#	define __GETARG_0( _0_, ... )		_0_
#	define __GETARG_1( _0_, _1_, ... )	_1_
#	define __CHECK_ERR( _expr_, _ret_ )	{if (not (_expr_)) { ASSERT(!(#_expr_)); return _ret_; } }
#	define CHECK_ERR( ... )				__CHECK_ERR( __GETARG_0( __VA_ARGS__ ), __GETARG_1( __VA_ARGS__, 0 ))
#	define RETURN_ERR( _msg_ )			{ ASSERT(!(#_msg_)); return 0; }
#	define CHECK( _expr_ )				{ ASSERT(_expr_); }

#	define BEGIN_ENUM_CHECKS()
#	define END_ENUM_CHECKS()

#   include "glslang-external.h"

// workaround an issue in the external lib
#include <type_traits>
template <class A, class B, class = std::enable_if_t<!std::is_same_v<A, B>> > auto Max(const A a, const B b) { return a > b ? a : b; }

#endif //!EXPORT_PPE_EXTERNAL_GLSL_TRACE
