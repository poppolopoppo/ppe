#pragma once

#include "RHI_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"
#include "Memory/UniquePtr.h"
#include "Misc/Event.h"
#include "Misc/Function.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
using FRHIInstance = class CONCAT3(F, TARGET_RHI, Instance);
using FRHIDevice = class CONCAT3(F, TARGET_RHI, Device);
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API FRHIModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    FRHIModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

public:
    static FRHIModule& Get(const FModularDomain& domain);

    using FRHIEvent = TFunction<void(RHI::ETargetRHI)>;
    using FInstanceEvent = TFunction<void(RHI::FRHIInstance&)>;
    using FDeviceEvent = TFunction<void(RHI::FRHIDevice&)>;

    PUBLIC_EVENT(OnInstancePreCreate, FRHIEvent);
    PUBLIC_EVENT(OnInstancePostCreate, FInstanceEvent);

    PUBLIC_EVENT(OnInstancePreDestroy, FInstanceEvent);
    PUBLIC_EVENT(OnInstancePostDestroy, FRHIEvent);

    PUBLIC_EVENT(OnDevicePreCreate, FInstanceEvent);
    PUBLIC_EVENT(OnDevicePostCreate, FDeviceEvent);

    PUBLIC_EVENT(OnDevicePreDestroy, FDeviceEvent);
    PUBLIC_EVENT(OnDevicePostDestroy, FInstanceEvent);

private:
    friend class RHI::CONCAT3(F, TARGET_RHI, Instance);
    friend class RHI::CONCAT3(F, TARGET_RHI, Device);

    void BroadcastInstancePreCreate_(RHI::ETargetRHI rhi);
    void BroadcastInstancePostCreate_(RHI::FRHIInstance& instance);

    void BroadcastInstancePreDestroy_(RHI::FRHIInstance& instance);
    void BroadcastInstancePostDestroy_(RHI::ETargetRHI rhi);

    void BroadcastDevicePreCreate_(RHI::FRHIInstance& instance);
    void BroadcastDevicePostCreate_(RHI::FRHIDevice& device);

    void BroadcastDevicePreDestroy_(RHI::FRHIDevice& device);
    void BroadcastDevicePostDestroy_(RHI::FRHIInstance& instance);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
