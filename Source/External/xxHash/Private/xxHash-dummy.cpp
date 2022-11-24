// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "xxHash-external.h"

// includes xxhash.c conditionally, simce it must be compiled if inlining is enabled
#if not (defined(XXH_INLINE_ALL) || defined(XXH_PRIVATE_API))
#   include "External/xxHash/xxHash.git/xxhash.c"
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
