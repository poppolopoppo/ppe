#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanShaderModule.h"

#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanShaderModule::FVulkanShaderModule(
    VkShaderModule vkShaderModule,
    FFingerprint sourceFingerprint,
    FStringView entryPoint
    ARGS_IF_RHIDEBUG(FStringView debugName) ) NOEXCEPT
:   _vkShaderModule(vkShaderModule)
,   _entryPoint(entryPoint)
#if USE_PPE_RHIDEBUG
,   _debugName(debugName.MakeView())
#endif
{
    // combine entry point with source fingerprint for fingerprint
    _fingerprint = Fingerprint128(_entryPoint.Str());
    Fingerprint128(&sourceFingerprint, sizeof(FFingerprint), _fingerprint);
}
//----------------------------------------------------------------------------
FVulkanShaderModule::~FVulkanShaderModule() NOEXCEPT {
    Assert_NoAssume(VK_NULL_HANDLE == _vkShaderModule); // must call TearDown() before destruction !
}
//----------------------------------------------------------------------------
void FVulkanShaderModule::TearDown(
    VkDevice vkDevice,
    PFN_vkDestroyShaderModule vkDestroyShaderModule,
    const VkAllocationCallbacks* pAllocator ) {
    if (_vkShaderModule) {
        Assert(VK_NULL_HANDLE != vkDevice);
        Assert(vkDestroyShaderModule);
        vkDestroyShaderModule(vkDevice, _vkShaderModule, pAllocator);
        _vkShaderModule = VK_NULL_HANDLE;
    }
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanShaderModule::ParseDebugOutput(TAppendable<FString>, EShaderDebugMode, FRawMemoryConst) {
    return false; // see FVulkanDebuggableShaderModule
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
