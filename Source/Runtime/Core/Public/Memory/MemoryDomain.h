#pragma once

#include "Core_fwd.h"

#include "IO/TextWriter_fwd.h"
#include "Memory/MemoryDomain_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if !USE_PPE_MEMORYDOMAINS
#   define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#else
#   define MEMORYDOMAIN_GROUP_IMPL(_Name, _Parent) /* groups are not publicly writable */ \
    namespace MemoryDomain { \
        struct MEMORYDOMAIN_NAME(_Name) { \
            static PPE_CORE_API const FMemoryTracking& TrackingData(); \
        }; \
    }
#   if WITH_PPE_MEMORYDOMAINS_COLLAPSING
#       define MEMORYDOMAIN_COLLAPSABLE_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#   endif
#   if !WITH_PPE_MEMORYDOMAINS_FULL_COLLAPSING
#       define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        struct MEMORYDOMAIN_NAME(_Name) { \
            static PPE_CORE_API FMemoryTracking& TrackingData(); \
        }; \
    }
#   else
#       define MEMORYDOMAIN_IMPL(_Name, _Parent) \
    namespace MemoryDomain { \
        using MEMORYDOMAIN_NAME(_Name) = MEMORYDOMAIN_NAME(_Parent); \
    }
#   endif
#endif

#include "Memory/MemoryDomain.Definitions-inl.h"

#undef MEMORYDOMAIN_COLLAPSABLE_IMPL
#undef MEMORYDOMAIN_GROUP_IMPL
#undef MEMORYDOMAIN_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API bool AllTrackingData(void* user, bool (*each)(void*, TMemoryView<const FMemoryTracking* const>)) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API void RegisterTrackingData(FMemoryTracking *pTrackingData);
PPE_CORE_API void UnregisterTrackingData(FMemoryTracking *pTrackingData);
//----------------------------------------------------------------------------
PPE_CORE_API void ReportAllocationFragmentation(FWTextWriter& oss);
PPE_CORE_API void ReportAllocationHistogram(FWTextWriter& oss);
//----------------------------------------------------------------------------
PPE_CORE_API void ReportAllTrackingData(FWTextWriter* optional = nullptr);
//----------------------------------------------------------------------------
PPE_CORE_API void ReportCsvTrackingData(FTextWriter* optional = nullptr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
