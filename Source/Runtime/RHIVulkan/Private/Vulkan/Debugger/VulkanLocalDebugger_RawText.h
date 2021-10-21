#pragma once

#include "Vulkan/Debugger/VulkanLocalDebugger.h"

#if !USE_PPE_RHIDEBUG
#   error "should not compile this file"
#endif

#include "Vulkan/Common/VulkanEnums.h"
#include "Vulkan/Common/VulkanEnumToString.h"
#include "Vulkan/Command/VulkanCommandBatch.h"

#include "RHI/EnumHelpers.h"
#include "RHI/EnumToString.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "Meta/Functor.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/RayTracing/VulkanRayTracingShaderTable.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanLocalDebugger::FRawTextDump_ {
    FStringBuilder& Out;
    mutable Fmt::FIndent Indent;
    const FVulkanLocalDebugger& Debugger;
    const FVulkanDevice& Device;

    FRawTextDump_(
        FStringBuilder* pout,
        const FVulkanLocalDebugger& debugger,
        const FVulkanDevice& device ) NOEXCEPT
    :   Out(*pout)
    ,   Indent(Fmt::FIndent::UsingTabs())
    ,   Debugger(debugger)
    ,   Device(device) {
        Assert(pout);
    }

    void DumpFrame(FStringView name, TMemoryView<const SVulkanCommandBatch> dependencies) const;

    void DumpImages() const;
    void DumpResourceInfo(const FVulkanImage* image, const FImageInfo& info) const;

    void DumpBuffers() const;
    void DumpResourceInfo(const FVulkanBuffer* buffer, const FBufferInfo& info) const;

    void DumpQueue(const FTaskMap& tasks) const;
    void DumpResourceUsage(const TMemoryView<const FResourceUsage>& usages) const;
    void DumpTaskData(PVulkanFrameTask task) const;

    void DumpTaskInfo(const FVulkanSubmitRenderPassTask& task) const;
    void DumpTaskInfo(const FVulkanDispatchComputeTask& task) const;
    void DumpTaskInfo(const FVulkanDispatchComputeIndirectTask& task) const;
    void DumpTaskInfo(const FVulkanCopyBufferTask& task) const;
    void DumpTaskInfo(const FVulkanCopyImageTask& task) const;
    void DumpTaskInfo(const FVulkanCopyBufferToImageTask& task) const;
    void DumpTaskInfo(const FVulkanCopyImageToBufferTask& task) const;
    void DumpTaskInfo(const FVulkanBlitImageTask& task) const;
    void DumpTaskInfo(const FVulkanResolveImageTask& task) const;
    void DumpTaskInfo(const FVulkanGenerateMipmapsTask& task) const;
    void DumpTaskInfo(const FVulkanFillBufferTask& task) const;
    void DumpTaskInfo(const FVulkanClearColorImageTask& task) const;
    void DumpTaskInfo(const FVulkanClearDepthStencilImageTask& task) const;
    void DumpTaskInfo(const FVulkanUpdateBufferTask& task) const;
    void DumpTaskInfo(const FVulkanPresentTask& task) const;
    void DumpTaskInfo(const FVulkanCustomTaskTask& task) const;
    void DumpTaskInfo(const FVulkanUpdateRayTracingShaderTableTask& task) const;
    void DumpTaskInfo(const FVulkanBuildRayTracingGeometryTask& task) const;
    void DumpTaskInfo(const FVulkanBuildRayTracingSceneTask& task) const;
    void DumpTaskInfo(const FVulkanTraceRaysTask& task) const;
};
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpFrame(
    FStringView name,
    TMemoryView<const SVulkanCommandBatch> dependencies) const {
    Out.clear();
    Out << Indent << "CommandBuffer {" << Eol;
    {
        Fmt::FIndent::FScope indentScope{ Indent };
        Out << Indent << "name: '" << name << "'" << Eol;

        DumpImages();
        DumpBuffers();

        if (not dependencies.empty()) {
            auto sep = Fmt::NotFirstTime(", ");
            Out << Indent << "dependsOn: [";
            for (const auto& dep : dependencies) {
                Out << sep << (dep->DebugName().empty()
                    ? MakeStringView("<no-name>")
                    : dep->DebugName().Str() );
            }
            Out << "]" << Eol;
        }

        Out << Indent << Fmt::Repeat('-', 79) << Eol;

        DumpQueue(Debugger._tasks);
    }

    Out << Indent << "}" << Eol
        << Indent << Fmt::Repeat('=', 79) << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpImages() const {
    STACKLOCAL_POD_ARRAY(const FImageResources::public_type*, sorted, Debugger._images.size());
    MakeIterable(Debugger._images)
        .Map([](const auto& img) NOEXCEPT { return &img; })
        .UninitializedCopyTo(sorted.begin());

    std::sort(sorted.begin(), sorted.end(),
        [](const auto* lhs, const auto* rhs) NOEXCEPT {
            if (lhs->first->DebugName() != rhs->first->DebugName())
                return lhs->first->DebugName().Less(rhs->first->DebugName());

            return (lhs->first < rhs->first);
        });

    for (const auto* img : sorted)
        DumpResourceInfo(img->first, img->second);
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpResourceInfo(const FVulkanImage* image, const FImageInfo& info) const {
    Out << Indent << "Image {" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };

        const FImageDesc& desc = image->Desc();

        Out << Indent << "name: '" << image->DebugName() << "'" << Eol
            << Indent << "type: " << desc.Type << Eol
            << Indent << "dimensions: " << desc.Dimensions << Eol
            << Indent << "format: " << desc.Format << Eol
            << Indent << "usage:" << desc.Usage << Eol
            << Indent << "arrayLayers: " << *desc.ArrayLayers << Eol
            << Indent << "maxLevel: " << *desc.MaxLevel << Eol
            << Indent << "samples: " << *desc.Samples << Eol;

        if (not info.Barriers.empty()) {
            Out << Indent << "barriers: {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                for (const auto& barrier : info.Barriers) {
                    Out << Indent << "ImageMemoryBarrier {" << Eol;
                    {
                        const Fmt::FIndent::FScope subSubIndent{ Indent };
                        Out << Indent << "srcTask: " << Debugger.TaskName_(barrier.SrcIndex) << Eol
                            << Indent << "dstTask: " << Debugger.TaskName_(barrier.DstIndex) << Eol
                            << Indent << "srcStageMask: " << barrier.SrcStageMask << Eol
                            << Indent << "dstStageMask: " << barrier.DstStageMask << Eol
                            << Indent << "dependencyFlags: " << barrier.DependencyFlags << Eol
                            << Indent << "srcAccessMask: " << barrier.Info.srcAccessMask << Eol
                            << Indent << "dstAccessMask: " << barrier.Info.dstAccessMask << Eol
                            << Indent << "srcQueueFamilyIndex: " << Debugger.QueueName_(Device, barrier.Info.srcQueueFamilyIndex) << Eol
                            << Indent << "dstQueueFamilyIndex: " << Debugger.QueueName_(Device, barrier.Info.dstQueueFamilyIndex) << Eol
                            << Indent << "oldLayout: " << barrier.Info.oldLayout << Eol
                            << Indent << "newLayout: " << barrier.Info.newLayout << Eol
                            << Indent << "aspectMask: " << barrier.Info.subresourceRange.aspectMask << Eol
                            << Indent << "baseMipLevel: " << barrier.Info.subresourceRange.baseMipLevel << Eol
                            << Indent << "levelCount: " << barrier.Info.subresourceRange.levelCount << Eol
                            << Indent << "baseArrayLevel: " << barrier.Info.subresourceRange.baseArrayLayer << Eol
                            << Indent << "layerCount: " << barrier.Info.subresourceRange.layerCount << Eol;
                     }
                    Out << Indent << '}' << Eol;
                }
            }
            Out << Indent << '}' << Eol << Eol;
        }
    }
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpBuffers() const {
    STACKLOCAL_POD_ARRAY(const FBufferResources::public_type*, sorted, Debugger._buffers.size());
    MakeIterable(Debugger._buffers)
        .Map([](const auto& buf) NOEXCEPT { return &buf; })
        .UninitializedCopyTo(sorted.begin());

    std::sort(sorted.begin(), sorted.end(),
        [](const auto* lhs, const auto* rhs) NOEXCEPT {
            if (lhs->first->DebugName() != rhs->first->DebugName())
                return lhs->first->DebugName().Less(rhs->first->DebugName());

            return (lhs->first < rhs->first);
        });

    for (const auto* buf : sorted)
        DumpResourceInfo(buf->first, buf->second);
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpResourceInfo(const FVulkanBuffer* buffer, const FBufferInfo& info) const {
    Out << Indent << "Buffer {" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };

        const FBufferDesc& desc = buffer->Desc();

        Out << Indent << "name: '" << buffer->DebugName() << "'" << Eol
            << Indent << "size: " << Fmt::SizeInBytes(desc.SizeInBytes) << Eol
            << Indent << "usage:" << desc.Usage << Eol;

        if (not info.Barriers.empty()) {
            Out << Indent << "barriers: {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                for (const auto& barrier : info.Barriers) {
                    Out << Indent << "BufferMemoryBarrier {" << Eol;
                    {
                        const Fmt::FIndent::FScope subSubIndent{ Indent };
                        Out << Indent << "srcTask: " << Debugger.TaskName_(barrier.SrcIndex) << Eol
                            << Indent << "dstTask: " << Debugger.TaskName_(barrier.DstIndex) << Eol
                            << Indent << "srcStageMask: " << barrier.SrcStageMask << Eol
                            << Indent << "dstStageMask: " << barrier.DstStageMask << Eol
                            << Indent << "dependencyFlags: " << barrier.DependencyFlags << Eol
                            << Indent << "srcAccessMask: " << barrier.Info.srcAccessMask << Eol
                            << Indent << "dstAccessMask: " << barrier.Info.dstAccessMask << Eol
                            << Indent << "srcQueueFamilyIndex: " << Debugger.QueueName_(Device, barrier.Info.srcQueueFamilyIndex) << Eol
                            << Indent << "dstQueueFamilyIndex: " << Debugger.QueueName_(Device, barrier.Info.dstQueueFamilyIndex) << Eol
                            << Indent << "offset: " << Fmt::Offset(barrier.Info.offset) << Eol
                            << Indent << "size: " << Fmt::SizeInBytes(barrier.Info.size) << Eol;
                     }
                    Out << Indent << '}' << Eol;
                }
            }
            Out << Indent << '}' << Eol << Eol;
        }
    }
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpQueue(const FTaskMap& tasks) const {
    auto sep = Fmt::NotLastTime(',', tasks.size());
    for (const auto& info : tasks) {
        if (not info.Task)
            continue;

        Out << Indent << "Task {" << Eol;
        {
            const Fmt::FIndent::FScope indentScope{ Indent };
            Out << Indent << "name: " << Debugger.TaskName_(info.Task) << Eol
                << Indent << "input = ["
                << Fmt::Join(info.Task->Inputs().Map(
                    [this](const PVulkanFrameTask& in) {
                        return Debugger.TaskName_(in);
                    }), ", ")
                << "]" << Eol
                << Indent << "output = ["
                << Fmt::Join(info.Task->Outputs().Map(
                    [this](const PVulkanFrameTask& out) {
                        return Debugger.TaskName_(out);
                    }), ", ")
                << "]" << Eol;

            DumpResourceUsage(info.Resources);
            DumpTaskData(info.Task);
        }
        Out << Indent << "}" << sep << Eol;
    }
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpResourceUsage(const TMemoryView<const FResourceUsage>& usages) const {
    struct FResourceInfo {
        FStringView Name;
        TPtrRef<const FResourceUsage> Usage;
    };

    if (usages.empty())
        return;

    STACKLOCAL_ASSUMEPOD_ARRAY(FResourceInfo, sorted, usages.size());
    usages.Map([](const FResourceUsage& usage) NOEXCEPT -> FResourceInfo {
        return Meta::Visit(usage,
            [&usage](const auto& x) NOEXCEPT -> FResourceInfo {
                return { x.first->DebugName(), &usage };
            });
    }).UninitializedCopyTo(sorted.begin());

    std::sort(sorted.begin(), sorted.end(),
        [](const FResourceInfo& lhs, const FResourceInfo& rhs) NOEXCEPT {
            return (lhs.Name != rhs.Name ? lhs.Name < rhs.Name : lhs.Usage < rhs.Usage);
        });

    Out << Indent << "Resource usages = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        for (const FResourceInfo& info : sorted) {
            Meta::Visit(*info.Usage,
                [&](const FImageUsage& image) {
                    Out << Indent << "ImageUsage {" << Eol;
                    {
                        const Fmt::FIndent::FScope subIndent{ Indent };
                        Out << Indent << "name: " << info.Name << Eol
                            << Indent << "usage: " << image.second.State << Eol
                            << Indent << "baseMipLevel: " << *image.second.Range.Mipmaps.begin() << Eol
                            << Indent << "levelCount: " << image.second.Range.Mipmaps.Extent() << Eol
                            << Indent << "baseArrayLayer: " << *image.second.Range.Layers.begin() << Eol
                            << Indent << "layerCount: " << image.second.Range.Layers.Extent() << Eol;
                    }
                    Out << Indent << "}" << Eol;
                },
                [&](const FBufferUsage& buffer) {
                    Out << Indent << "BufferUsage {" << Eol;
                    {
                        const Fmt::FIndent::FScope subIndent{ Indent };
                        Out << Indent << "name: " << info.Name << Eol
                            << Indent << "usage: " << buffer.second.State << Eol
                            << Indent << "offset: " << Fmt::Offset(*buffer.second.Range.begin()) << Eol
                            << Indent << "size: " << Fmt::SizeInBytes(buffer.second.Range.Extent()) << Eol;
                    }
                    Out << Indent << "}" << Eol;
                },
                [&](const FRTSceneUsage& scene) {
                    Out << Indent << "RTSceneUsage {" << Eol;
                    {
                        const Fmt::FIndent::FScope subIndent{ Indent };
                        Out << Indent << "name: " << info.Name << Eol
                            << Indent << "usage: " << scene.second.State << Eol
                            << Indent << "flags: " << scene.first->Flags() << Eol
                            << Indent << "maxInstanceCount: " << scene.first->MaxInstanceCount() << Eol;
                    }
                    Out << Indent << "}" << Eol;
                },
                [&](const FRTGeometryUsage& geometry) {
                    Out << Indent << "RTGeometryUsage {" << Eol;
                    {
                        const Fmt::FIndent::FScope subIndent{ Indent };
                        Out << Indent << "name: " << info.Name << Eol
                            << Indent << "usage: " << geometry.second.State << Eol
                            << Indent << "flags: " << geometry.first->Flags() << Eol
                            << Indent << "aabbCount: " << geometry.first->Aabbs().size() << Eol
                            << Indent << "triangleCount: " << geometry.first->Triangles().size() << Eol;
                    }
                    Out << Indent << "}" << Eol;
                });
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanSubmitRenderPassTask& task) const {
    UNUSED(task);
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanDispatchComputeTask& task) const {
    Out << Indent << "pipeline: '" << task.Pipeline->DebugName() << "'" << Eol;
    if (task.LocalGroupSize.has_value())
        Out << Indent << "localGroupSize: " << *task.LocalGroupSize << Eol;
    Out << Indent << "commands = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Commands.size());
        for (const auto& cmd : task.Commands)
            Out << Indent << "{ " << cmd.BaseGroup << ", " << cmd.GroupCount << " }" << sep << Eol;
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanDispatchComputeIndirectTask& task) const {
    Out << Indent << "pipeline: '" << task.Pipeline->DebugName() << "'" << Eol
        << Indent << "indirectBuffer: '" << task.IndirectBuffer->DebugName() << "'" << Eol;
    if (task.LocalGroupSize.has_value())
        Out << Indent << "localGroupSize: " << *task.LocalGroupSize << Eol;
    Out << Indent << "commands = [ "
        << Fmt::Join(task.Commands.MakeView().Map(
            [](const FDispatchComputeIndirect::FComputeCommand& cmd) {
                return cmd.IndirectBufferOffset;
            }), ", ")
        << " ]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanCopyBufferTask& task) const {
    Out << Indent << "srcBuffer: '" << task.SrcBuffer->DebugName() << "'" << Eol
        << Indent << "dstBuffer: '" << task.DstBuffer->DebugName() << "'" << Eol
        << Indent << "regions = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions)
            Out << Indent
                << "{ " << Fmt::Offset(span.SrcOffset)
                << ", " << Fmt::Offset(span.DstOffset)
                << ", " << Fmt::SizeInBytes(span.Size)
                << " }" << sep << Eol;
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanCopyImageTask& task) const {
    Out << Indent << "srcImage: '" << task.SrcImage->DebugName() << "'" << Eol
        << Indent << "srcLayout: " << task.SrcLayout << Eol
        << Indent << "dstImage: '" << task.DstImage->DebugName() << "'" << Eol
        << Indent << "dstLayout: " << task.DstLayout << Eol
        << Indent << "regions = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << "src.aspectMask: " << span.SrcSubresource.AspectMask << Eol
                    << Indent << "src.mipLevel: " << *span.SrcSubresource.MipLevel << Eol
                    << Indent << "src.baseLayer: " << *span.SrcSubresource.BaseLayer << Eol
                    << Indent << "src.layerCount: " << span.SrcSubresource.LayerCount << Eol
                    << Indent << "src.offset: " << span.SrcOffset << Eol
                    << Indent << "dst.aspectMask: " << span.DstSubresource.AspectMask << Eol
                    << Indent << "dst.mipLevel: " << *span.DstSubresource.MipLevel << Eol
                    << Indent << "dst.baseLayer: " << *span.DstSubresource.BaseLayer << Eol
                    << Indent << "dst.layerCount: " << span.DstSubresource.LayerCount << Eol
                    << Indent << "dst.offset: " << span.DstOffset << Eol
                    << Indent << "size: " << span.Size << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanCopyBufferToImageTask& task) const {
    Out << Indent << "srcBuffer: '" << task.SrcBuffer->DebugName() << "'" << Eol
        << Indent << "dstImage: '" << task.DstImage->DebugName() << "'" << Eol
        << Indent << "dstLayout: " << task.DstLayout << Eol
        << Indent << "regions = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << "bufferOffset: " << span.BufferOffset << Eol
                    << Indent << "bufferRowLength: " << span.BufferRowLength << Eol
                    << Indent << "bufferImageHeight: " << span.BufferImageHeight << Eol
                    << Indent << "imageAspectMask: " << span.ImageLayers.AspectMask << Eol
                    << Indent << "imageMipLevel: " << *span.ImageLayers.MipLevel << Eol
                    << Indent << "imageBaseArrayLayer: " << *span.ImageLayers.BaseLayer << Eol
                    << Indent << "imageLayerCount: " << span.ImageLayers.LayerCount << Eol
                    << Indent << "imageOffset: " << span.ImageOffset << Eol
                    << Indent << "imageSize: " << span.ImageSize << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanCopyImageToBufferTask& task) const {
    Out << Indent << "srcImage: '" << task.SrcImage->DebugName() << "'" << Eol
        << Indent << "srcLayout: " << task.SrcLayout << Eol
        << Indent << "dstBuffer: '" << task.DstBuffer->DebugName() << "'" << Eol
        << Indent << "regions = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << "imageAspectMask: " << span.ImageLayers.AspectMask << Eol
                    << Indent << "imageMipLevel: " << *span.ImageLayers.MipLevel << Eol
                    << Indent << "imageBaseArrayLayer: " << *span.ImageLayers.BaseLayer << Eol
                    << Indent << "imageLayerCount: " << span.ImageLayers.LayerCount << Eol
                    << Indent << "imageOffset: " << span.ImageOffset << Eol
                    << Indent << "imageSize: " << span.ImageSize << Eol
                    << Indent << "bufferOffset: " << span.BufferOffset << Eol
                    << Indent << "bufferRowLength: " << span.BufferRowLength << Eol
                    << Indent << "bufferImageHeight: " << span.BufferImageHeight << Eol;

            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanBlitImageTask& task) const {
    Out << Indent << "srcImage: '" << task.SrcImage->DebugName() << "'" << Eol
        << Indent << "srcLayout: " << task.SrcLayout << Eol
        << Indent << "dstImage: '" << task.DstImage->DebugName() << "'" << Eol
        << Indent << "dstLayout: " << task.DstLayout << Eol
        << Indent << "regions = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << "src.aspectMask: " << span.SrcSubresource.AspectMask << Eol
                    << Indent << "src.mipLevel: " << *span.SrcSubresource.MipLevel << Eol
                    << Indent << "src.baseLayer: " << *span.SrcSubresource.BaseLayer << Eol
                    << Indent << "src.layerCount: " << span.SrcSubresource.LayerCount << Eol
                    << Indent << "src.offset0: " << span.SrcOffset0 << Eol
                    << Indent << "src.offset1: " << span.SrcOffset1 << Eol
                    << Indent << "dst.aspectMask: " << span.DstSubresource.AspectMask << Eol
                    << Indent << "dst.mipLevel: " << *span.DstSubresource.MipLevel << Eol
                    << Indent << "dst.baseLayer: " << *span.DstSubresource.BaseLayer << Eol
                    << Indent << "dst.layerCount: " << span.DstSubresource.LayerCount << Eol
                    << Indent << "dst.offset0: " << span.DstOffset0 << Eol
                    << Indent << "dst.offset1: " << span.DstOffset1 << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanResolveImageTask& task) const {
    Out << Indent << "srcImage: '" << task.SrcImage->DebugName() << "'" << Eol
        << Indent << "srcLayout: " << task.SrcLayout << Eol
        << Indent << "dstImage: '" << task.DstImage->DebugName() << "'" << Eol
        << Indent << "dstLayout: " << task.DstLayout << Eol
        << Indent << "regions = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << "src.aspectMask: " << span.SrcSubresource.AspectMask << Eol
                    << Indent << "src.mipLevel: " << *span.SrcSubresource.MipLevel << Eol
                    << Indent << "src.baseLayer: " << *span.SrcSubresource.BaseLayer << Eol
                    << Indent << "src.layerCount: " << span.SrcSubresource.LayerCount << Eol
                    << Indent << "src.offset: " << span.SrcOffset << Eol
                    << Indent << "dst.aspectMask: " << span.DstSubresource.AspectMask << Eol
                    << Indent << "dst.mipLevel: " << *span.DstSubresource.MipLevel << Eol
                    << Indent << "dst.baseLayer: " << *span.DstSubresource.BaseLayer << Eol
                    << Indent << "dst.layerCount: " << span.DstSubresource.LayerCount << Eol
                    << Indent << "dst.offset: " << span.DstOffset << Eol
                    << Indent << "extent: " << span.Extent << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanFillBufferTask& task) const {
    Out << Indent << "dstBuffer: '" << task.DstBuffer->DebugName() << "'" << Eol
        << Indent << "dstOffset: " << Fmt::Offset(task.DstOffset) << Eol
        << Indent << "size: " << Fmt::SizeInBytes(task.Size) << Eol
        << Indent << "pattern: " << Fmt::Offset(task.Pattern) << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanClearColorImageTask& task) const {
    Out << Indent << "dstImage: '" << task.DstImage->DebugName() << "'" << Eol
        << Indent << "dstLayout: " << task.DstLayout << Eol
        << Indent << "ranges = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Ranges.size());
        for (const auto& span : task.Ranges) {
            Out << Indent << "Range {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << "aspectMask: " << span.AspectMask << Eol
                    << Indent << "mipLevel: " << *span.BaseMipLevel << Eol
                    << Indent << "levelCount: " << span.LevelCount << Eol
                    << Indent << "baseLayer: " << *span.BaseLayer << Eol
                    << Indent << "layerCount: " << span.LayerCount << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanClearDepthStencilImageTask& task) const {
    Out << Indent << "dstImage: '" << task.DstImage->DebugName() << "'" << Eol
        << Indent << "dstLayout: " << task.DstLayout << Eol
        << Indent << "clearValue.depth: " << task.ClearValue.depth << Eol
        << Indent << "clearValue.stencil: " << task.ClearValue.stencil << Eol
        << Indent << "ranges = [" << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Ranges.size());
        for (const auto& span : task.Ranges) {
            Out << Indent << "Range {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << "aspectMask: " << span.AspectMask << Eol
                    << Indent << "mipLevel: " << *span.BaseMipLevel << Eol
                    << Indent << "levelCount: " << span.LevelCount << Eol
                    << Indent << "baseLayer: " << *span.BaseLayer << Eol
                    << Indent << "layerCount: " << span.LayerCount << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanUpdateBufferTask& task) const {
    Out << Indent << "dstBuffer: '" << task.DstBuffer->DebugName() << "'" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanPresentTask& task) const {
    Out << Indent << "srcImage: '" << task.SrcImage->DebugName() << "'" << Eol
        << Indent << "layer: " << *task.Layer << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanBuildRayTracingGeometryTask& task) const {
    Out << Indent << "geometry: " << task.RTGeometry->DebugName() << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanBuildRayTracingSceneTask& task) const {
    Out << Indent << "scene: " << task.RTScene->DebugName() << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanUpdateRayTracingShaderTableTask& task) const {
    Out << Indent << "scene: " << task.RTScene->DebugName() << Eol
        << Indent << "shaderTable: " << task.ShaderTable->DebugName() << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanTraceRaysTask& task) const {
    Out << Indent << "groupCount: " << task.GroupCount << Eol
        << Indent << "shaderTable: " << task.ShaderTable->DebugName() << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanGenerateMipmapsTask& task) const {
    Out << Indent << "image: " << task.Image->DebugName() << Eol
        << Indent << "baseLayer: " << task.BaseLayer << Eol
        << Indent << "layerCount: " << task.LayerCount << Eol
        << Indent << "baseMipLevel: " << task.BaseMipLevel << Eol
        << Indent << "levelCount: " << task.LevelCount << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanCustomTaskTask& task) const {
    Out << Indent << "buffers: [ "
        << Fmt::Join(task.Buffers.Map(
        [](const TPair<const FVulkanLocalBuffer*, EResourceState>& in) {
            return in.first->DebugName();
        }), ", ") << " ]" << Eol
        << Indent << "images: [ "
        << Fmt::Join(task.Images.Map(
        [](const TPair<const FVulkanLocalImage*, EResourceState>& in) {
            return in.first->DebugName();
        }), ", ") << " ]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskData(PVulkanFrameTask task) const {
    Assert(task);

    Meta::Visit(task->DebugRef(),
        [this](const auto* pTask) {
            Assert(pTask);
            DumpTaskInfo(*pTask);
        });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
