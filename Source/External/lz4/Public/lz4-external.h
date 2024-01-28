#pragma once

#include "Core_fwd.h"

#ifndef EXPORT_PPE_EXTERNAL_LZ4
PRAGMA_MSVC_WARNING_PUSH()
#   pragma push_macro("ALLOCATOR")
#   pragma push_macro("FORCE_INLINE")
#   pragma push_macro("XXH_NAMESPACE")
#endif

#undef ALLOCATOR
#undef FORCE_INLINE

#define XXH_NAMESPACE LZ4_
#define LZ4_USER_MEMORY_FUNCTIONS

PRAGMA_MSVC_WARNING_DISABLE(4244) // conversion from 'XXX' to 'YYY', possible loss of data
PRAGMA_MSVC_WARNING_DISABLE(4505) // 'XXX' unreferenced local function has been removed
PRAGMA_MSVC_WARNING_DISABLE(5033) // 'register' is no longer a supported storage class (C++17)
PRAGMA_MSVC_WARNING_DISABLE(6237) // (<zero> && <expression>) is always zero.  <expression> is never evaluated and might have side effects
PRAGMA_MSVC_WARNING_DISABLE(6239) // (<non-zero constant> && <expression>) always evaluates to the result of <expression>.  Did you intend to use the bitwise-and operator?
PRAGMA_MSVC_WARNING_DISABLE(6262) // function uses '16456' bytes of stack. consider moving some data to the heap.

#ifdef __clang__
#    pragma clang system_header
#endif

#ifdef __gcc__
#    pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_LZ4

#   include "External/lz4/lz4.git/lib/lz4.h"
#   include "External/lz4/lz4.git/lib/lz4hc.h"

#   undef XXH_NAMESPACE

// clean the mess done by lz4-config.h :

#   pragma pop_macro("ALLOCATOR")
#   pragma pop_macro("FORCE_INLINE")
#   pragma pop_macro("XXH_NAMESPACE")

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_LZ4
