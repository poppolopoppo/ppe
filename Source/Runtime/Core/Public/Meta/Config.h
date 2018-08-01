#pragma once

//----------------------------------------------------------------------------
// Global parameter defining how much cores we handle at max
//----------------------------------------------------------------------------
#define PPE_MAX_NUMPPES           64 // Change with care !

//----------------------------------------------------------------------------
// Global switch for turning of assertions
//----------------------------------------------------------------------------
#define USE_PPE_ASSERTIONS         1 //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Global switch for handling or throwing exceptions
//----------------------------------------------------------------------------
#define USE_PPE_EXCEPTIONS         1 //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Global switch for using Assert() even when it should be disabled
//----------------------------------------------------------------------------
#define USE_PPE_FORCE_ASSERTION    0 //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Global switch for using LOG() event it should be disabled
//----------------------------------------------------------------------------
#define USE_PPE_FORCE_LOGGING      0 //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Global switch for using stomp allocator, memory guards and turning special allocators off
//----------------------------------------------------------------------------
#define USE_PPE_MEMORY_DEBUGGING   0 //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Determined from build configuration
//----------------------------------------------------------------------------
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