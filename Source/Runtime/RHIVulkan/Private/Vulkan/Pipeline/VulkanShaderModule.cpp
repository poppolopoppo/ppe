﻿#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanShaderModule.h"

#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanShaderModule::FVulkanShaderModule(
    VkShaderModule vkShaderModule,
    hash_t sourceFingerprint,
    FStringView entryPoint
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) NOEXCEPT
:   _vkShaderModule(vkShaderModule)
,   _entryPoint(entryPoint)
#if USE_PPE_RHIDEBUG
,   _debugName(debugName.MakeView())
#endif
{
    // combine entry point with source fingerprint for hash value
    _hashValue = hash_string(_entryPoint);
    hash_combine(_hashValue, sourceFingerprint);
}
//----------------------------------------------------------------------------
FVulkanShaderModule::~FVulkanShaderModule() NOEXCEPT {
    Assert_NoAssume(VK_NULL_HANDLE == _vkShaderModule); // must call TearDown() before destruction !
}
//----------------------------------------------------------------------------
void FVulkanShaderModule::TearDown(const FVulkanDevice& device) {
    if (_vkShaderModule) {
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