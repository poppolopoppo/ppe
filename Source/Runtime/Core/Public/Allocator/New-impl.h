#pragma once

#include "Allocator/New.h"

#if PPE_OVERRIDE_NEW_OPERATORS

#   if PPE_OVERRIDE_NEW_IN_MODULE
#       include "Allocator/New.Definitions-inl.h"
#   endif

#endif //!PPE_OVERRIDE_NEW_OPERATORS
