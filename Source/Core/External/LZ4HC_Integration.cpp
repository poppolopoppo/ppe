#include "stdafx.h"

// this file is isolated so it's not a problem to do this :
#undef ALLOCATOR
#undef FORCE_INLINE

#ifdef PLATFORM_WINDOWS
#   pragma warning(push)
#   pragma warning(disable: 6239) // (<non-zero constant> && <expression>) always evaluates to the result of <expression>.  Did you intend to use the bitwise-and operator?
#endif

#include "External/lz4hc.c"

#ifdef PLATFORM_WINDOWS
#   pragma warning(pop)
#endif
