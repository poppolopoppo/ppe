#pragma once

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_CORE_EXTERNAL_XXHASH
    PRAGMA_MSVC_WARNING_PUSH()
#   pragma push_macro("FORCE_INLINE")
#endif

#undef FORCE_INLINE
#define XXH_NAMESPACE xxHash

#ifndef EXPORT_CORE_EXTERNAL_XXHASH

#   include "Core.External/xxHash/xxhash.h"

// clean the mess done by xxHash-config.h :

#   pragma pop_macro("FORCE_INLINE")

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_CORE_EXTERNAL_XXHASH