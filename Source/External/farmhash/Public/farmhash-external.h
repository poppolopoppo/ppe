#pragma once

#include "Meta/Warnings.h"

#ifndef EXPORT_PPE_EXTERNAL_FARMHASH
    PRAGMA_MSVC_WARNING_PUSH()
#endif

#define FARMHASH_NO_BUILTIN_EXPECT
#define FARMHASH_ASSUME_SSE3
#define FARMHASH_ASSUME_SSE41
#define FARMHASH_ASSUME_SSE42
//#define FARMHASH_ASSUME_AESNI
#define FARMHASH_ASSUME_AVX
#define FARMHASH_ASSUME_AVX2
#define FARMHASH_CAN_USE_CXX11
#define FARMHASH_NO_CXX_STRING
#define NAMESPACE_FOR_HASH_FUNCTIONS FarmHash

PRAGMA_MSVC_WARNING_DISABLE(4127) // conditional expression is constant
PRAGMA_MSVC_WARNING_DISABLE(4244) // 'argument': conversion from 'uint64_t' to 'uint32_t', possible loss of data
PRAGMA_MSVC_WARNING_DISABLE(4267) // 'initializing' : conversion from 'size_t' to 'uint32_t', possible loss of data
PRAGMA_MSVC_WARNING_DISABLE(4307) // '*' : integral constant overflow
PRAGMA_MSVC_WARNING_DISABLE(4319) // '~': zero extending 'uint32_t' to 'uint64_t' of greater size
PRAGMA_MSVC_WARNING_DISABLE(4456) // declaration of 'XXX' hides previous local declaration
PRAGMA_MSVC_WARNING_DISABLE(6246) // Local declaration of 'XXX' hides declaration of the same name in outer scope.
PRAGMA_MSVC_WARNING_DISABLE(6297) // Arithmetic overflow:  32-bit value is shifted, then cast to 64-bit value.  Results might not be an expected value.
PRAGMA_MSVC_WARNING_DISABLE(6313) // Incorrect operator:  zero-valued flag cannot be tested with bitwise-and.  Use an equality test to check for zero-valued flags.

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_FARMHASH

#   include "External/farmhash/farmhash.git/src/farmhash.h"

// clean the mess done by farmhash.h :

#   undef FARMHASH_NO_BUILTIN_EXPECT
#   undef FARMHASH_ASSUME_SSE3
#   undef FARMHASH_ASSUME_SSE41
#   undef FARMHASH_ASSUME_SSE42
//# undef FARMHASH_ASSUME_AESNI
#   undef FARMHASH_ASSUME_AVX
#   undef FARMHASH_CAN_USE_CXX11
#   undef FARMHASH_NO_CXX_STRING
#   undef NAMESPACE_FOR_HASH_FUNCTIONS

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_FARMHASH
