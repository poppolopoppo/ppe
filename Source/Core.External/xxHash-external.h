#pragma once

#ifndef EXPORT_CORE_EXTERNAL_XXHASH
#   pragma warning(push)
#   pragma push_macro("FORCE_INLINE")
#endif

#undef FORCE_INLINE
#define XXH_NAMESPACE xxHash

#ifndef EXPORT_CORE_EXTERNAL_XXHASH

#   include "Core.External/xxHash/xxhash.h"

// clean the mess done by xxHash-config.h :

#   undef XXH_NAMESPACE
#   pragma pop_macro("FORCE_INLINE")

#   pragma warning(pop)

#endif //!EXPORT_CORE_EXTERNAL_XXHASH
