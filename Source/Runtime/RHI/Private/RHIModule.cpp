#include "stdafx.h"

#include "RHIModule.h"

#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"

#include "HAL/RHIDevice.h"
#include "HAL/RHIInstance.h"
#include "HAL/TargetRHI.h"

#include "BuildModules.generated.h"

namespace PPE {
namespace RHI {
LOG_CATEGORY(PPE_RHI_API, RHI)
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FRHIModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FRHIModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList )
};
//----------------------------------------------------------------------------
FRHIModule& FRHIModule::Get(const FModularDomain& domain) {
    return domain.ModuleChecked<FRHIModule>(STRINGIZE(BUILD_TARGET_NAME));
}
//----------------------------------------------------------------------------
FRHIModule::FRHIModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FRHIModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);
    RHI::FInstance::ParseOptions();
}
//----------------------------------------------------------------------------
void FRHIModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);
}
//----------------------------------------------------------------------------
void FRHIModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FRHIModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

}
//----------------------------------------------------------------------------
void FRHIModule::BroadcastInstancePreCreate_(RHI::ETargetRHI rhi) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast pre-create instance event for {0}RHI", rhi);
    _OnInstancePreCreate(rhi);
}
//----------------------------------------------------------------------------
void FRHIModule::BroadcastInstancePostCreate_(RHI::FRHIInstance& instance) {
    using namespace RHI;
    LOG(RHI, Emphasis, L"broadcast post-create instance event for {0}RHI <{1}>", instance.TargetRHI(), Fmt::Pointer(&instance));
    _OnInstancePostCreate(instance);
}
//----------------------------------------------------------------------------
void FRHIModule::BroadcastInstancePreDestroy_(RHI::FRHIInstance& instance) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast pre-destroy instance event for {0}RHI <{1}>", instance.TargetRHI(), Fmt::Pointer(&instance));
    _OnInstancePreDestroy(instance);
}
//----------------------------------------------------------------------------
void FRHIModule::BroadcastInstancePostDestroy_(RHI::ETargetRHI rhi) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast post-destroy instance event for {0}RHI", rhi);
    _OnInstancePostDestroy(rhi);
}
//----------------------------------------------------------------------------
void FRHIModule::BroadcastDevicePreCreate_(RHI::FRHIInstance& instance) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast pre-create device event for {0}RHI with instance <{1}>", instance.TargetRHI(), Fmt::Pointer(&instance));
    _OnDevicePreCreate(instance);
}
//----------------------------------------------------------------------------
void FRHIModule::BroadcastDevicePostCreate_(RHI::FRHIDevice& device) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast post-create device event for {0}RHI <{2} with instance <{1}>", device.Instance().TargetRHI(), Fmt::Pointer(&device.Instance()), Fmt::Pointer(&device));
    _OnDevicePostCreate(device);
}
//----------------------------------------------------------------------------
void FRHIModule::BroadcastDevicePreDestroy_(RHI::FRHIDevice& device) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast pre-destroy device event for {0}RHI <{2} with instance <{1}>", device.Instance().TargetRHI(), Fmt::Pointer(&device.Instance()), Fmt::Pointer(&device));
    _OnDevicePreDestroy(device);
}
//----------------------------------------------------------------------------
void FRHIModule::BroadcastDevicePostDestroy_(RHI::FRHIInstance& instance) {
    using namespace RHI;
    LOG(RHI, Info, L"broadcast post-destroy device event for {0}RHI with instance <{1}>", instance.TargetRHI(), Fmt::Pointer(&instance));
    _OnDevicePostDestroy(instance);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
