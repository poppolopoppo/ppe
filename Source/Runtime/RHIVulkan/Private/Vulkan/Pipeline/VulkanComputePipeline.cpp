
#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanComputePipeline.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVulkanComputePipeline::FPipelineInstance::Invalidate() NOEXCEPT {
#if 1
    HashValue = hash_tuple(
        LayoutId, LocalGroupSize, Flags
        ARGS_IF_RHIDEBUG(DebugMode) );
#else
    STATIC_ASSERT(offsetof(FPipelineInstance, HashValue) == 0);
    STATIC_ASSERT(Meta::TCheckSameSize<FPipelineInstance, std::aligned_storage_t<
        sizeof(LocalGroupSize) +
        sizeof(HashValue) +
        sizeof(LayoutId) +
        sizeof(Flags) ARG0_IF_RHIDEBUG(+sizeof(DebugMode))
        >>::value );

    return hash_mem(&LayoutId, sizeof(FPipelineInstance) - sizeof(HashValue));
#endif
}
//----------------------------------------------------------------------------
FVulkanComputePipeline::~FVulkanComputePipeline() {
    ONLY_IF_RHIDEBUG(Assert_NoAssume(_pipeline.LockExclusive()->Shaders.empty()));
    ONLY_IF_RHIDEBUG(FReadWriteLock::FScopeLockWrite scopeWrite{ _instanceRWLock });
    ONLY_IF_RHIDEBUG(Assert_NoAssume(_instanceMap.empty()));
}
//----------------------------------------------------------------------------
bool FVulkanComputePipeline::Construct(const FComputePipelineDesc& desc, FRawPipelineLayoutID layoutId ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(layoutId);

    const auto exclusive = _pipeline.LockExclusive();

    ONLY_IF_RHIDEBUG(_debugName = debugName);

    exclusive->BaseLayoutId = FPipelineLayoutID{ layoutId };
    exclusive->DefaultLocalGroupSize = desc.DefaultLocalGroupSize;
    exclusive->LocalSizeSpecialization = desc.LocalSizeSpecialization;

    for (auto& src : desc.Shader.Sources) {
        const PShaderModule& moduleRef = std::get<PShaderModule>(*src.second->Data());
        Assert(moduleRef);

        exclusive->Shaders.Push(
            checked_cast<FVulkanShaderModule>(moduleRef)
            ARGS_IF_RHIDEBUG(EShaderDebugMode_From(src.first)) );
    }

    return (not exclusive->Shaders.empty());
}
//----------------------------------------------------------------------------
void FVulkanComputePipeline::TearDown(FVulkanResourceManager& resources) {
    const FVulkanDevice& device = resources.Device();

    const auto exclusive = _pipeline.LockExclusive();

    {
        const FReadWriteLock::FScopeLockWrite lockInstanceW{ _instanceRWLock };

        for (auto& ppln : _instanceMap) {
            device.vkDestroyPipeline(device.vkDevice(), ppln.second, device.vkAllocator());
            resources.ReleaseResource(ppln.first.LayoutId);
        }

        _instanceMap.clear();
    }

    if (exclusive->BaseLayoutId)
        resources.ReleaseResource(exclusive->BaseLayoutId.Release());

    exclusive->Shaders.clear();
    ONLY_IF_RHIDEBUG(_debugName.Clear());

    exclusive->BaseLayoutId = Default;
    exclusive->DefaultLocalGroupSize = uint3::Zero;
    exclusive->LocalSizeSpecialization = uint3{ FComputePipelineDesc::UndefinedSpecialization };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
