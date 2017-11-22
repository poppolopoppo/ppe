#pragma once

//----------------------------------------------------------------------------
// Global switch for handling or throwing exceptions
//----------------------------------------------------------------------------
#define USE_CORE_EXCEPTIONS         1 //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Global switch for using Assert() even when it should be disabled
//----------------------------------------------------------------------------
#define USE_CORE_FORCE_ASSERTION    0 //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Global switch for using LOG() event it should be disabled
//----------------------------------------------------------------------------
#define USE_CORE_FORCE_LOGGING      0 //%_NOCOMMIT%

//----------------------------------------------------------------------------
// Global switch for using stomp allocator, memory guards and turning special allocators off
//----------------------------------------------------------------------------
#define USE_CORE_MEMORY_DEBUGGING   0 //%_NOCOMMIT%
