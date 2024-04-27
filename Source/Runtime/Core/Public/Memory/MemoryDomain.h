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
struct ITrackingDataObserver {
    virtual void OnRegisterTrackingData(FMemoryTracking* pTrackingData) = 0;
    virtual void OnUnregisterTrackingData(FMemoryTracking* pTrackingData) = 0;
};
PPE_CORE_API void RegisterTrackingDataObserver(ITrackingDataObserver* pObserver);
PPE_CORE_API void UnregisterTrackingDataObserver(ITrackingDataObserver* pObserver);
//----------------------------------------------------------------------------
PPE_CORE_API bool AllTrackingDataSorted(void* user, bool (*each)(void*, TMemoryView<const FMemoryTracking* const>)) NOEXCEPT;
PPE_CORE_API bool ForeachTrackingDataUnsorted(void* user, bool (*each)(void*, const FMemoryTracking&)) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_CORE_API void RegisterTrackingData(FMemoryTracking *pTrackingData);
PPE_CORE_API void UnregisterTrackingData(FMemoryTracking *pTrackingData);
//----------------------------------------------------------------------------
PPE_CORE_API void ReportAllocationFragmentation(FTextWriter& oss);
PPE_CORE_API void ReportAllocationHistogram(FTextWriter& oss);
//----------------------------------------------------------------------------
PPE_CORE_API void ReportAllTrackingData(FTextWriter* optional = nullptr);
//----------------------------------------------------------------------------
PPE_CORE_API void ReportCsvTrackingData(FTextWriter* optional = nullptr);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
