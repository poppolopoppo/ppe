﻿#pragma once

#include "Vulkan/Command/VulkanTaskProcessor.h"

#include "Vulkan/Instance/VulkanDevice.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FVulkanTaskProcessor::FPipelineResourceBarriers::FPipelineResourceBarriers(
    FVulkanTaskProcessor& processor,
    TMemoryView<const u32> dynamicOffsets ) NOEXCEPT
:   _processor(processor)
,   _dynamicOffsets(dynamicOffsets)
{}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FPipelineResourceBarriers::operator()(const FUniformID& , const FPipelineResources::FBuffer& buffer) {
    for (const auto& elem : buffer.Elements.MakeView()) {
        const FVulkanLocalBuffer* const pLocalBuffer = _processor.ToLocal_(elem.BufferId);
        if (not Ensure(pLocalBuffer))
            continue;

        const auto localBuf = pLocalBuffer->Read();

        const VkDeviceSize offset = static_cast<VkDeviceSize>(elem.Offset +
            (buffer.DynamicOffsetIndex < _dynamicOffsets.size()
                ? _dynamicOffsets[buffer.DynamicOffsetIndex]
                : 0 ));

        const VkDeviceSize size = static_cast<VkDeviceSize>(
            (elem.Size == UMax
                ? localBuf->SizeInBytes() - offset
                : elem.Size ));

#if USE_PPE_RHIDEBUG
        // validation

        Assert_NoAssume(size >= buffer.StaticSize);
        Assert_NoAssume(buffer.ArrayStride == 0 or Meta::IsAlignedPow2(buffer.ArrayStride, checked_cast<size_t>(size) - buffer.StaticSize));
        Assert_NoAssume(offset < localBuf->SizeInBytes());
        Assert_NoAssume(offset + size <= localBuf->SizeInBytes());

        auto& limits = _processor._workerCmd->Device().Limits();
        Unused(limits);
        Assert_NoAssume(Meta::IsAlignedPow2(
            checked_cast<size_t>(limits.minUniformBufferOffsetAlignment),
            checked_cast<size_t>(offset)));

        if (buffer.State.OnlyState() == EResourceState_UniformRead) {
            Assert_NoAssume(size == buffer.StaticSize);
            Assert_NoAssume(size <= limits.maxUniformBufferRange);
        }
        else {
            Assert_NoAssume(size <= limits.maxStorageBufferRange);
        }
#endif

        _processor.AddBuffer_(pLocalBuffer, buffer.State, offset, size);
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FPipelineResourceBarriers::operator()(const FUniformID& , const FPipelineResources::FTexelBuffer& texelBuffer) {
    for (const auto& elem : texelBuffer.Elements.MakeView()) {
        const FVulkanLocalBuffer* const pLocalBuffer = _processor.ToLocal_(elem.BufferId);
        if (not Ensure(pLocalBuffer))
            continue;

        const auto localBuf = pLocalBuffer->Read();

        const VkDeviceSize offset = static_cast<VkDeviceSize>(elem.Desc.Offset);
        const VkDeviceSize size = static_cast<VkDeviceSize>(
            (elem.Desc.SizeInBytes == UMax
                ? localBuf->SizeInBytes() - offset
                : elem.Desc.SizeInBytes ));

        _processor.AddBuffer_(pLocalBuffer, texelBuffer.State, offset, size);
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FPipelineResourceBarriers::operator()(const FUniformID& , const FPipelineResources::FImage& image) {
    for (const auto& elem : image.Elements.MakeView()) {
        const FVulkanLocalImage* const pLocalImage = _processor.ToLocal_(elem.ImageId);

        if (Ensure(pLocalImage)) {
            _processor.AddImage_(pLocalImage,
                image.State,
                EResourceState_ToImageLayout(image.State, pLocalImage->Read()->AspectMask),
                *elem.Desc );
        }
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FPipelineResourceBarriers::operator()(const FUniformID& , const FPipelineResources::FTexture& texture) {
    for (const auto& elem : texture.Elements.MakeView()) {
        const FVulkanLocalImage* const pLocalImage = _processor.ToLocal_(elem.ImageId);

        if (Ensure(pLocalImage)) {
            _processor.AddImage_(pLocalImage, texture.State,
                EResourceState_ToImageLayout(texture.State, pLocalImage->Read()->AspectMask),
                *elem.Desc );
        }
    }
}
//----------------------------------------------------------------------------
inline void FVulkanTaskProcessor::FPipelineResourceBarriers::operator()(const FUniformID& , const FPipelineResources::FRayTracingScene& scene) {
    for (const auto& elem : scene.Elements.MakeView()) {
        const FVulkanRTLocalScene* const pLocalScene = _processor.ToLocal_(elem.SceneId);
        if (not Ensure(pLocalScene))
            continue;

        _processor.AddRTScene_(pLocalScene, EResourceState_RayTracingShaderRead);

        const auto sharedData = pLocalScene->GlobalData()->SharedData();
        Assert_NoAssume(not sharedData->GeometryInstances.empty());

        for (auto& instance : sharedData->GeometryInstances) {
            if (const FVulkanRTLocalGeometry* pLocalGeom = _processor.ToLocal_(*instance.GeometryId))
                _processor.AddRTGeometry_(pLocalGeom, EResourceState_RayTracingShaderRead);
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
