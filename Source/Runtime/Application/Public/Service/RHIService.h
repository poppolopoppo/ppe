#pragma once

#include "Application_fwd.h"

#include "RHI_fwd.h"

#include "Memory/UniquePtr.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(RHIService);
class IRHIService {
public:
    IRHIService() = default;
    virtual ~IRHIService() = default;

    virtual RHI::FDevice* CreateMainDevice(FWindowRHI* window) = 0;
    virtual void DestroyMainDevice(FWindowRHI* window, RHI::FDevice* device) = 0;

    virtual RHI::FDevice* CreateHeadlessDevice(bool computeOnly) = 0;
    virtual void DestroyHeadlessDevice(RHI::FDevice* device) = 0;

    virtual RHI::FDevice* MainDevice() const NOEXCEPT = 0;
    virtual void SetMainDevice(RHI::FDevice* device) = 0;

public:
    static PPE_APPLICATION_API void Make(URHIService* pRHI, RHI::ETargetRHI rhi);
    static PPE_APPLICATION_API void MakeDefault(URHIService* pRHI);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
