#pragma once

//----------------------------------------------------------------------------
// Global parameter defining how much cores we handle at max
//----------------------------------------------------------------------------
#ifndef PPE_MAX_NUMCPUCORE
#   define PPE_MAX_NUMCPUCORE         64 // Change with care !
#endif

//----------------------------------------------------------------------------
// Global switch for turning of assertions
//----------------------------------------------------------------------------
#ifndef USE_PPE_ASSERTIONS
#   define USE_PPE_ASSERTIONS         1 //%_NOCOMMIT%
#endif

//----------------------------------------------------------------------------
// Global switch for handling or throwing exceptions
//----------------------------------------------------------------------------
#ifndef USE_PPE_EXCEPTIONS
#   define USE_PPE_EXCEPTIONS         1 //%_NOCOMMIT%
#endif

//----------------------------------------------------------------------------
// Global switch for using Assert() even when it should be disabled
//----------------------------------------------------------------------------
#ifndef USE_PPE_FORCE_ASSERTION
#   define USE_PPE_FORCE_ASSERTION    0 //%_NOCOMMIT%
#endif

//----------------------------------------------------------------------------
// Global switch for using LOG() event it should be disabled
//----------------------------------------------------------------------------
#ifndef USE_PPE_FORCE_LOGGING
#   define USE_PPE_FORCE_LOGGING      0 //%_NOCOMMIT%
#endif

//----------------------------------------------------------------------------
// Global switch for using address sanitize, enable memory debugging with std malloc instead of stomp
//----------------------------------------------------------------------------
#ifndef USE_PPE_SANITIZER
#   define USE_PPE_SANITIZER          0 //%_NOCOMMIT%
#endif

//----------------------------------------------------------------------------
// Global switch for using stomp allocator, memory guards and turning special allocators off
//----------------------------------------------------------------------------
#ifndef USE_PPE_MEMORY_DEBUGGING
#   define USE_PPE_MEMORY_DEBUGGING   (USE_PPE_SANITIZER) //%_NOCOMMIT%
#endif

//----------------------------------------------------------------------------
// Determined from build configuration
//----------------------------------------------------------------------------
#if defined(_DEBUG) && !defined(NDEBUG)
#   define USE_PPE_DEBUG 1
#else
#   define USE_PPE_DEBUG 0
#endif
#if defined(FASTDEBUG)
#   define USE_PPE_FASTDEBUG 1
#else
#   define USE_PPE_FASTDEBUG 0
#endif
#ifdef PROFILING_ENABLED
#   define USE_PPE_PROFILING 1
#else
#   define USE_PPE_PROFILING 0
#endif
#ifdef FINAL_RELEASE
#   define USE_PPE_FINAL_RELEASE 1
#else
#   define USE_PPE_FINAL_RELEASE 0
#endif
#ifndef PPE_HAS_CXXRTTI
#   define PPE_HAS_CXXRTTI 0
#endif
