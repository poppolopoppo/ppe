#pragma once

#include "Core.h"

#ifdef EXPORT_PPE_RTTI
#   define PPE_RTTI_API DLL_EXPORT
#else
#   define PPE_RTTI_API DLL_IMPORT
#endif

#if !defined(FINAL_RELEASE) && !defined(PROFILING_ENABLED)
#   define USE_PPE_RTTI_CHECKS 1
#else
#   define USE_PPE_RTTI_CHECKS 0
#endif

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RTTI_API FRTTIModule : public FModule {
public:
    FRTTIModule();
    virtual ~FRTTIModule();

protected:
    virtual void Start(FModuleManager& manager) override final;
    virtual void Shutdown() override final;
    virtual void ReleaseMemory() override final;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
