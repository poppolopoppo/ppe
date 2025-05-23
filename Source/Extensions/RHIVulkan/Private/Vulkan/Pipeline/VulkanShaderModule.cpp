﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
    FStringView entryPoint ) NOEXCEPT
:   _vkShaderModule(vkShaderModule)
,   _entryPoint(entryPoint)
{
    // combine entry point with source fingerprint for fingerprint
    _fingerprint = Fingerprint128(_entryPoint.Str());
    _fingerprint = Fingerprint128(&sourceFingerprint, sizeof(FFingerprint), _fingerprint);
}
//----------------------------------------------------------------------------
FVulkanShaderModule::~FVulkanShaderModule() NOEXCEPT {
    Assert_NoAssume(VK_NULL_HANDLE == _vkShaderModule); // must call TearDown() before destruction !
}
//----------------------------------------------------------------------------
bool FVulkanShaderModule::Construct(const FVulkanDevice& device ARGS_IF_RHIDEBUG(FStringView debugName)) {
#if USE_PPE_RHIDEBUG
    _debugName = debugName.MakeView();
    if (_vkShaderModule && _debugName)
        device.SetObjectName(_vkShaderModule, _debugName.c_str(), VK_OBJECT_TYPE_SHADER_MODULE);
#else
    Unused(device);
#endif
    return true;
}
//----------------------------------------------------------------------------
void FVulkanShaderModule::TearDown(const FVulkanDevice& device) {
    if (_vkShaderModule) {
#if USE_PPE_RHIDEBUG
        device.SetObjectName(_vkShaderModule, nullptr, VK_OBJECT_TYPE_SHADER_MODULE);
#endif

        device.vkDestroyShaderModule(device.vkDevice(), _vkShaderModule, device.vkAllocator());

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
