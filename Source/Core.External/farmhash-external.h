#pragma once

#ifndef EXPORT_CORE_EXTERNAL_FARMHASH
#   pragma warning(push)
#endif

#define FARMHASH_NO_BUILTIN_EXPECT
#define FARMHASH_ASSUME_SSE3
#define FARMHASH_ASSUME_SSE41
#define FARMHASH_ASSUME_SSE42
//#define FARMHASH_ASSUME_AESNI
#define FARMHASH_ASSUME_AVX
#define FARMHASH_CAN_USE_CXX11
#define FARMHASH_NO_CXX_STRING
#define NAMESPACE_FOR_HASH_FUNCTIONS FarmHash

#ifdef _MSC_VER
#   pragma warning(disable: 4244) // 'argument': conversion from 'uint64_t' to 'uint32_t', possible loss of data
#   pragma warning(disable: 4267) // 'initializing' : conversion from 'size_t' to 'uint32_t', possible loss of data
#   pragma warning(disable: 4307) // '*' : integral constant overflow
#   pragma warning(disable: 4319) // '~': zero extending 'uint32_t' to 'uint64_t' of greater size
#   pragma warning(disable: 6297) // Arithmetic overflow:  32-bit value is shifted, then cast to 64-bit value.  Results might not be an expected value.
#endif

#ifndef EXPORT_CORE_EXTERNAL_FARMHASH

#   include "Core.External/farmhash/src/farmhash.h"

// clean the mess done by xxHash-config.h :

#   undef FARMHASH_NO_BUILTIN_EXPECT
#   undef FARMHASH_ASSUME_SSE3
#   undef FARMHASH_ASSUME_SSE41
#   undef FARMHASH_ASSUME_SSE42
//# undef FARMHASH_ASSUME_AESNI
#   undef FARMHASH_ASSUME_AVX
#   undef FARMHASH_CAN_USE_CXX11
#   undef FARMHASH_NO_CXX_STRING
#   undef NAMESPACE_FOR_HASH_FUNCTIONS

#   pragma warning(pop)

#endif //!EXPORT_CORE_EXTERNAL_FARMHASH
