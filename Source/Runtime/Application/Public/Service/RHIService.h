#pragma once

#include "Application_fwd.h"

#include "RHI_fwd.h"
#include "HAL/TargetRHI_fwd.h"

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

    virtual RHI::PFrameGraph CreateDefaultFrameGraph(FWindowRHI* window) = 0;
    virtual void DestroyDefaultFrameGraph(FWindowRHI* window, RHI::PFrameGraph& frameGraph) = 0;

    virtual RHI::PFrameGraph CreateHeadlessFrameGraph(bool computeOnly) = 0;
    virtual void DestroyHeadlessFrameGraph(RHI::PFrameGraph& frameGraph) = 0;

    virtual RHI::PFrameGraph MainFrameGraph() const NOEXCEPT = 0;
    virtual void SetMainFrameGraph(RHI::PFrameGraph&& frameGraph) = 0;

public:
    void CreateMainFrameGraph(FWindowRHI* window);
    void DestroyMainFrameGraph(FWindowRHI* window);

    static PPE_APPLICATION_API void Make(URHIService* pRHI, ETargetRHI rhi);
    static PPE_APPLICATION_API void MakeDefault(URHIService* pRHI);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
