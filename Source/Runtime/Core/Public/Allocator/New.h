#pragma once

#ifndef PPE_OVERRIDE_NEW_OPERATORS
#   define PPE_OVERRIDE_NEW_OPERATORS (!USE_PPE_SANITIZER) // turn to 0 to disable global allocator overriding %_NOCOMMIT%
#endif

#if PPE_OVERRIDE_NEW_OPERATORS

#   if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
#       define PPE_ISOLATE_NEW_OPERATORS 1 // compiled in separate TU, no forward declaration
#   else
#       define PPE_ISOLATE_NEW_OPERATORS 0 // defined inline from this header
#   endif

#   ifdef BUILD_LINK_DYNAMIC
#       define PPE_OVERRIDE_NEW_IN_MODULE (PPE_ISOLATE_NEW_OPERATORS) // defined in each New-impl.h
#   else
#       define PPE_OVERRIDE_NEW_IN_MODULE 0 // defined only on Core/ModuleExport.cpp
#   endif

#   define PPE_OVERRIDE_NEW_ONCE (PPE_ISOLATE_NEW_OPERATORS and not PPE_OVERRIDE_NEW_IN_MODULE)

#   if !PPE_ISOLATE_NEW_OPERATORS
#       include "Allocator/New.Definitions-inl.h"
#   endif

#endif //!PPE_OVERRIDE_NEW_OPERATORS
