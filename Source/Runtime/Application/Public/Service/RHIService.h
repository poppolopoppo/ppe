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

    virtual RHI::PFrameGraph CreateMainFrameGraph(FWindowRHI* window) = 0;
    virtual void DestroyMainFrameGraph(FWindowRHI* window, RHI::PFrameGraph& device) = 0;

    virtual RHI::PFrameGraph CreateHeadlessFrameGraph(bool computeOnly) = 0;
    virtual void DestroyHeadlessFrameGraph(RHI::PFrameGraph& device) = 0;

    virtual RHI::PFrameGraph MainFrameGraph() const NOEXCEPT = 0;
    virtual void SetMainFrameGraph(RHI::PFrameGraph& device) = 0;

public:
    static PPE_APPLICATION_API void Make(URHIService* pRHI, RHI::ETargetRHI rhi);
    static PPE_APPLICATION_API void MakeDefault(URHIService* pRHI);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
