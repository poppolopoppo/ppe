#pragma once

#define PPE_COMPILER_MESSAGE(_Message) \
    _Pragma(STRINGIZE(message(_Message)))

#define PPE_COMPILER_WARNING(_Code, _Message) PPE_COMPILER_MESSAGE("warning " _Code ": " _Message)
#define PPE_COMPILER_ERROR(_Code, _Message) PPE_COMPILER_MESSAGE("error " _Code ": " _Message)
#define PPE_DEPRECATED __attribute__((deprecated))

#if USE_PPE_PLATFORM_DEBUG

#   if __has_builtin(__builtin_debugtrap)
#      define PPE_DEBUG_BREAK() __builtin_debugtrap()
#   else
#      include <signal.h>
#      if defined(SIGTRAP)
#          define PPE_DEBUG_BREAK() raise(SIGTRAP)
#      else
#          define PPE_DEBUG_BREAK() __asm__ volatile("int $0x03")
#      endif
#   endif

#   if __has_builtin(__builtin_trap)
#      define PPE_DEBUG_CRASH() __builtin_trap()
#   endif

#   define PPE_DECLSPEC_CODE_SECTION(_NAME) __attribute__((section(_NAME)))

#endif
