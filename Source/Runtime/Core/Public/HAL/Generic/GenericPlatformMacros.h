#pragma once

#include <cstdlib>

#ifndef PPE_COMPILER_MESSAGE
#   define PPE_COMPILER_MESSAGE(_Message)
#endif

#ifndef PPE_DEPRECATED
#   define PPE_DEPRECATED
#endif

#ifndef PRAGMA_MSVC_WARNING_PUSH
#   define PRAGMA_MSVC_WARNING_PUSH()
#endif
#ifndef PRAGMA_MSVC_WARNING_DISABLE
#   define PRAGMA_MSVC_WARNING_DISABLE(_WARNING_CODE)
#endif
#ifndef PRAGMA_MSVC_WARNING_POP
#   define PRAGMA_MSVC_WARNING_POP()
#endif

#ifndef PPE_DEBUG_BREAK
#   define PPE_DEBUG_BREAK() NOOP()
#endif
#ifndef PPE_DEBUG_CRASH
#   define PPE_DEBUG_CRASH() std::abort()
#endif
#ifndef PPE_DECLSPEC_ALLOCATOR
#   define PPE_DECLSPEC_ALLOCATOR()
#endif
#ifndef PPE_DECLSPEC_CODE_SECTION
#   define PPE_DECLSPEC_CODE_SECTION(_NAME)
#endif
