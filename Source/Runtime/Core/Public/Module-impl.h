#pragma once

#ifndef PPE_STATICMODULES_STARTUP
#   error "can't be included first !"
#endif

PRAGMA_INITSEG_LIB

#include "Allocator/New-impl.h"

PPE_WARNING("PPE", "compiling module <" STRINGIZE(PPE_STATICMODULES_STARTUP) "> ...")
