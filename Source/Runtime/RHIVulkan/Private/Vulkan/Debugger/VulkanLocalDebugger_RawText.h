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

private: // formatting helpers
    STATIC_CONST_INTEGRAL(size_t, FieldWidth, 17);

    template <size_t _Dim>
    static auto Align_(const char(&text)[_Dim]) NOEXCEPT {
        return Fmt::PadRight(text, FieldWidth);
    }

};
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpFrame(
    FStringView name,
    TMemoryView<const SVulkanCommandBatch> dependencies) const {
    Out.clear();
    Out << Indent << "CommandBuffer {" << Eol;
    {
        Fmt::FIndent::FScope indentScope{ Indent };
        Out << Indent << Align_("name:") << Fmt::Quoted(name, '"') << Eol;

        DumpImages();
        DumpBuffers();

        if (not dependencies.empty()) {
            auto sep = Fmt::NotFirstTime(", ");
            Out << Indent << Align_("dependsOn:") << '[';
            for (const auto& dep : dependencies) {
                Out << sep << Fmt::Quoted(dep->DebugName().empty()
                    ? MakeStringView("<no-name>")
                    : dep->DebugName().Str(), '"');
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

        Out << Indent << Align_("name:") << Fmt::Quoted(image->DebugName(), '"') << Eol
            << Indent << Align_("type:") << desc.Type << Eol
            << Indent << Align_("dimensions:") << desc.Dimensions << Eol
            << Indent << Align_("format:") << desc.Format << Eol
            << Indent << Align_("usage:") << desc.Usage << Eol
            << Indent << Align_("arrayLayers:") << *desc.ArrayLayers << Eol
            << Indent << Align_("maxLevel:") << *desc.MaxLevel << Eol
            << Indent << Align_("samples:") << *desc.Samples << Eol;

        if (not info.Barriers.empty()) {
            Out << Indent << Align_("barriers:") << '[' << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                for (const auto& barrier : info.Barriers) {
                    Out << Indent << "ImageMemoryBarrier {" << Eol;
                    {
                        const Fmt::FIndent::FScope subSubIndent{ Indent };
                        Out << Indent << Align_("srcTask:") << Fmt::Quoted(Debugger.TaskName_(barrier.SrcIndex), '"') << Eol
                            << Indent << Align_("dstTask:") << Fmt::Quoted(Debugger.TaskName_(barrier.DstIndex), '"') << Eol
                            << Indent << Align_("srcStageMask:") << static_cast<VkPipelineStageFlagBits>(barrier.SrcStageMask) << Eol
                            << Indent << Align_("dstStageMask:") << static_cast<VkPipelineStageFlagBits>(barrier.DstStageMask) << Eol
                            << Indent << Align_("dependencyFlags:") << static_cast<VkDependencyFlagBits>(barrier.DependencyFlags) << Eol
                            << Indent << Align_("srcAccessMask:") << static_cast<VkAccessFlagBits>(barrier.Info.srcAccessMask) << Eol
                            << Indent << Align_("dstAccessMask:") << static_cast<VkAccessFlagBits>(barrier.Info.dstAccessMask) << Eol
                            << Indent << Align_("oldLayout:") << barrier.Info.oldLayout << Eol
                            << Indent << Align_("newLayout:") << barrier.Info.newLayout << Eol
                            << Indent << Align_("aspectMask:") << static_cast<VkImageAspectFlagBits>(barrier.Info.subresourceRange.aspectMask) << Eol
                            << Indent << Align_("baseMipLevel:") << barrier.Info.subresourceRange.baseMipLevel << Eol
                            << Indent << Align_("levelCount:") << barrier.Info.subresourceRange.levelCount << Eol
                            << Indent << Align_("baseArrayLayer:") << barrier.Info.subresourceRange.baseArrayLayer << Eol
                            << Indent << Align_("layerCount:") << barrier.Info.subresourceRange.layerCount << Eol
                            << Indent << Align_("srcQueueFamily:") << Debugger.QueueName_(Device, barrier.Info.srcQueueFamilyIndex) << Eol
                            << Indent << Align_("dstQueueFamily:") << Debugger.QueueName_(Device, barrier.Info.dstQueueFamilyIndex) << Eol;
                     }
                    Out << Indent << '}' << Eol;
                }
            }
            Out << Indent << ']' << Eol << Eol;
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

        Out << Indent << Align_("name:") << Fmt::Quoted(buffer->DebugName(), '"') << Eol
            << Indent << Align_("size:") << Fmt::SizeInBytes(desc.SizeInBytes) << Eol
            << Indent << Align_("usage:") << desc.Usage << Eol;

        if (not info.Barriers.empty()) {
            Out << Indent << Align_("barriers:") << '[' << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                for (const auto& barrier : info.Barriers) {
                    Out << Indent << "BufferMemoryBarrier {" << Eol;
                    {
                        const Fmt::FIndent::FScope subSubIndent{ Indent };
                        Out << Indent << Align_("srcTask:") << Fmt::Quoted(Debugger.TaskName_(barrier.SrcIndex), '"') << Eol
                            << Indent << Align_("dstTask:") << Fmt::Quoted(Debugger.TaskName_(barrier.DstIndex), '"') << Eol
                            << Indent << Align_("srcStageMask:") << static_cast<VkPipelineStageFlagBits>(barrier.SrcStageMask) << Eol
                            << Indent << Align_("dstStageMask:") << static_cast<VkPipelineStageFlagBits>(barrier.DstStageMask) << Eol
                            << Indent << Align_("dependencyFlags:") << static_cast<VkDependencyFlagBits>(barrier.DependencyFlags) << Eol
                            << Indent << Align_("srcAccessMask:") << static_cast<VkAccessFlagBits>(barrier.Info.srcAccessMask) << Eol
                            << Indent << Align_("dstAccessMask:") << static_cast<VkAccessFlagBits>(barrier.Info.dstAccessMask) << Eol
                            << Indent << Align_("offset:") << Fmt::Offset(barrier.Info.offset) << Eol
                            << Indent << Align_("size:") << Fmt::SizeInBytes(barrier.Info.size) << Eol
                            << Indent << Align_("srcQueueFamily:") << Debugger.QueueName_(Device, barrier.Info.srcQueueFamilyIndex) << Eol
                            << Indent << Align_("dstQueueFamily:") << Debugger.QueueName_(Device, barrier.Info.dstQueueFamilyIndex) << Eol;
                     }
                    Out << Indent << '}' << Eol;
                }
            }
            Out << Indent << ']' << Eol << Eol;
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
            Out << Indent << Align_("name:")
                << Fmt::Quoted(Debugger.TaskName_(info.Task), '"') << Eol
                << Indent << Align_("input:") << '['
                << Fmt::Join(info.Task->Inputs().Map(
                    [this](const PVulkanFrameTask& in) {
                        return Fmt::Quoted(Debugger.TaskName_(in), '"');
                    }), ", ")
                << "]" << Eol
                << Indent << Align_("output:") << '['
                << Fmt::Join(info.Task->Outputs().Map(
                    [this](const PVulkanFrameTask& out) {
                        return Fmt::Quoted(Debugger.TaskName_(out), '"');
                    }), ", ")
                << "]" << Eol;

            DumpResourceUsage(info.Resources);
            //DumpTaskData(info.Task); // #TODO: see FVulkanCommandBuffer::FlushLocalResourceStates_(), which already deleted all tasks used by this graph
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

    Out << Indent << Align_("resourceUsages:") << '[' << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        for (const FResourceInfo& info : sorted) {
            Meta::Visit(*info.Usage,
                [&](const FImageUsage& image) {
                    Out << Indent << "ImageUsage {" << Eol;
                    {
                        const Fmt::FIndent::FScope subIndent{ Indent };
                        Out << Indent << Align_("name:") << Fmt::Quoted(info.Name, '"') << Eol
                            << Indent << Align_("usage:") << image.second.State << Eol
                            << Indent << Align_("baseMipLevel:") << *image.second.Range.Mipmaps.begin() << Eol
                            << Indent << Align_("levelCount:") << image.second.Range.Mipmaps.Extent() << Eol
                            << Indent << Align_("baseArrayLayer:") << *image.second.Range.Layers.begin() << Eol
                            << Indent << Align_("layerCount:") << image.second.Range.Layers.Extent() << Eol;
                    }
                    Out << Indent << "}" << Eol;
                },
                [&](const FBufferUsage& buffer) {
                    Out << Indent << "BufferUsage {" << Eol;
                    {
                        const Fmt::FIndent::FScope subIndent{ Indent };
                        Out << Indent << Align_("name:") << Fmt::Quoted(info.Name, '"') << Eol
                            << Indent << Align_("usage:") << buffer.second.State << Eol
                            << Indent << Align_("offset:") << Fmt::Offset(*buffer.second.Range.begin()) << Eol
                            << Indent << Align_("size:") << Fmt::SizeInBytes(buffer.second.Range.Extent()) << Eol;
                    }
                    Out << Indent << "}" << Eol;
                },
                [&](const FRTSceneUsage& scene) {
                    Out << Indent << "RTSceneUsage {" << Eol;
                    {
                        const Fmt::FIndent::FScope subIndent{ Indent };
                        Out << Indent << Align_("name:") << Fmt::Quoted(info.Name, '"') << Eol
                            << Indent << Align_("usage:") << scene.second.State << Eol
                            << Indent << Align_("flags:") << scene.first->Flags() << Eol
                            << Indent << Align_("maxInstanceCount:") << scene.first->MaxInstanceCount() << Eol;
                    }
                    Out << Indent << "}" << Eol;
                },
                [&](const FRTGeometryUsage& geometry) {
                    Out << Indent << "RTGeometryUsage {" << Eol;
                    {
                        const Fmt::FIndent::FScope subIndent{ Indent };
                        Out << Indent << Align_("name:") << Fmt::Quoted(info.Name, '"') << Eol
                            << Indent << Align_("usage:") << geometry.second.State << Eol
                            << Indent << Align_("flags:") << geometry.first->Flags() << Eol
                            << Indent << Align_("aabbCount:") << geometry.first->Aabbs().size() << Eol
                            << Indent << Align_("triangleCount:") << geometry.first->Triangles().size() << Eol;
                    }
                    Out << Indent << "}" << Eol;
                });
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanSubmitRenderPassTask& task) const {
    Unused(task);
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanDispatchComputeTask& task) const {
    Out << Indent << Align_("pipeline:") << Fmt::Quoted(task.Pipeline->DebugName(), '"') << Eol;
    if (task.LocalGroupSize.has_value())
        Out << Indent << Align_("localGroupSize:") << *task.LocalGroupSize << Eol;
    Out << Indent << Align_("commands:") << '[' << Eol;
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
    Out << Indent << Align_("pipeline:") << Fmt::Quoted(task.Pipeline->DebugName(), '"') << Eol
        << Indent << Align_("indirectBuffer:") << Fmt::Quoted(task.IndirectBuffer->DebugName(), '"') << Eol;
    if (task.LocalGroupSize.has_value())
        Out << Indent << Align_("localGroupSize:") << *task.LocalGroupSize << Eol;
    Out << Indent << "commands = [ "
        << Fmt::Join(task.Commands.MakeView().Map(
            [](const FDispatchComputeIndirect::FComputeCommand& cmd) {
                return cmd.IndirectBufferOffset;
            }), ", ")
        << " ]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanCopyBufferTask& task) const {
    Out << Indent << Align_("srcBuffer:") << Fmt::Quoted(task.SrcBuffer->DebugName(), '"') << Eol
        << Indent << Align_("dstBuffer:") << Fmt::Quoted(task.DstBuffer->DebugName(), '"') << Eol
        << Indent << Align_("regions:") << '[' << Eol;
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
    Out << Indent << Align_("srcImage:") << Fmt::Quoted(task.SrcImage->DebugName(), '"') << Eol
        << Indent << Align_("srcLayout:") << task.SrcLayout << Eol
        << Indent << Align_("dstImage:") << Fmt::Quoted(task.DstImage->DebugName(), '"') << Eol
        << Indent << Align_("dstLayout:") << task.DstLayout << Eol
        << Indent << Align_("regions:") << '[' << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << Align_("src.aspectMask:") << span.SrcSubresource.AspectMask << Eol
                    << Indent << Align_("src.mipLevel:") << *span.SrcSubresource.MipLevel << Eol
                    << Indent << Align_("src.baseLayer:") << *span.SrcSubresource.BaseLayer << Eol
                    << Indent << Align_("src.layerCount:") << span.SrcSubresource.LayerCount << Eol
                    << Indent << Align_("src.offset:") << span.SrcOffset << Eol
                    << Indent << Align_("dst.aspectMask:") << span.DstSubresource.AspectMask << Eol
                    << Indent << Align_("dst.mipLevel:") << *span.DstSubresource.MipLevel << Eol
                    << Indent << Align_("dst.baseLayer:") << *span.DstSubresource.BaseLayer << Eol
                    << Indent << Align_("dst.layerCount:") << span.DstSubresource.LayerCount << Eol
                    << Indent << Align_("dst.offset:") << span.DstOffset << Eol
                    << Indent << Align_("size:") << span.Size << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanCopyBufferToImageTask& task) const {
    Out << Indent << Align_("srcBuffer:") << Fmt::Quoted(task.SrcBuffer->DebugName(), '"') << Eol
        << Indent << Align_("dstImage:") << Fmt::Quoted(task.DstImage->DebugName(), '"') << Eol
        << Indent << Align_("dstLayout:") << task.DstLayout << Eol
        << Indent << Align_("regions:") << '[' << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << Align_("bufferOffset:") << span.BufferOffset << Eol
                    << Indent << Align_("bufferRowLength:") << span.BufferRowLength << Eol
                    << Indent << Align_("bufferImageHeight:") << span.BufferImageHeight << Eol
                    << Indent << Align_("imageAspectMask:") << span.ImageLayers.AspectMask << Eol
                    << Indent << Align_("imageMipLevel:") << *span.ImageLayers.MipLevel << Eol
                    << Indent << Align_("imageBaseArrayLayer:") << *span.ImageLayers.BaseLayer << Eol
                    << Indent << Align_("imageLayerCount:") << span.ImageLayers.LayerCount << Eol
                    << Indent << Align_("imageOffset:") << span.ImageOffset << Eol
                    << Indent << Align_("imageSize:") << span.ImageSize << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanCopyImageToBufferTask& task) const {
    Out << Indent << Align_("srcImage:") << Fmt::Quoted(task.SrcImage->DebugName(), '"') << Eol
        << Indent << Align_("srcLayout:") << task.SrcLayout << Eol
        << Indent << Align_("dstBuffer:") << Fmt::Quoted(task.DstBuffer->DebugName(), '"') << Eol
        << Indent << Align_("regions:") << '[' << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << Align_("imageAspectMask:") << span.ImageLayers.AspectMask << Eol
                    << Indent << Align_("imageMipLevel:") << *span.ImageLayers.MipLevel << Eol
                    << Indent << Align_("imageBaseArrayLayer:") << *span.ImageLayers.BaseLayer << Eol
                    << Indent << Align_("imageLayerCount:") << span.ImageLayers.LayerCount << Eol
                    << Indent << Align_("imageOffset:") << span.ImageOffset << Eol
                    << Indent << Align_("imageSize:") << span.ImageSize << Eol
                    << Indent << Align_("bufferOffset:") << span.BufferOffset << Eol
                    << Indent << Align_("bufferRowLength:") << span.BufferRowLength << Eol
                    << Indent << Align_("bufferImageHeight:") << span.BufferImageHeight << Eol;

            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanBlitImageTask& task) const {
    Out << Indent << Align_("srcImage:") << Fmt::Quoted(task.SrcImage->DebugName(), '"') << Eol
        << Indent << Align_("srcLayout:") << task.SrcLayout << Eol
        << Indent << Align_("dstImage:") << Fmt::Quoted(task.DstImage->DebugName(), '"') << Eol
        << Indent << Align_("dstLayout:") << task.DstLayout << Eol
        << Indent << Align_("regions:") << '[' << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << Align_("src.aspectMask:") << span.SrcSubresource.AspectMask << Eol
                    << Indent << Align_("src.mipLevel:") << *span.SrcSubresource.MipLevel << Eol
                    << Indent << Align_("src.baseLayer:") << *span.SrcSubresource.BaseLayer << Eol
                    << Indent << Align_("src.layerCount:") << span.SrcSubresource.LayerCount << Eol
                    << Indent << Align_("src.offset0:") << span.SrcOffset0 << Eol
                    << Indent << Align_("src.offset1:") << span.SrcOffset1 << Eol
                    << Indent << Align_("dst.aspectMask:") << span.DstSubresource.AspectMask << Eol
                    << Indent << Align_("dst.mipLevel:") << *span.DstSubresource.MipLevel << Eol
                    << Indent << Align_("dst.baseLayer:") << *span.DstSubresource.BaseLayer << Eol
                    << Indent << Align_("dst.layerCount:") << span.DstSubresource.LayerCount << Eol
                    << Indent << Align_("dst.offset0:") << span.DstOffset0 << Eol
                    << Indent << Align_("dst.offset1:") << span.DstOffset1 << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanResolveImageTask& task) const {
    Out << Indent << Align_("srcImage:") << Fmt::Quoted(task.SrcImage->DebugName(), '"') << Eol
        << Indent << Align_("srcLayout:") << task.SrcLayout << Eol
        << Indent << Align_("dstImage:") << Fmt::Quoted(task.DstImage->DebugName(), '"') << Eol
        << Indent << Align_("dstLayout:") << task.DstLayout << Eol
        << Indent << Align_("regions:") << '[' << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Regions.size());
        for (const auto& span : task.Regions) {
            Out << Indent << "Region {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << Align_("src.aspectMask:") << span.SrcSubresource.AspectMask << Eol
                    << Indent << Align_("src.mipLevel:") << *span.SrcSubresource.MipLevel << Eol
                    << Indent << Align_("src.baseLayer:") << *span.SrcSubresource.BaseLayer << Eol
                    << Indent << Align_("src.layerCount:") << span.SrcSubresource.LayerCount << Eol
                    << Indent << Align_("src.offset:") << span.SrcOffset << Eol
                    << Indent << Align_("dst.aspectMask:") << span.DstSubresource.AspectMask << Eol
                    << Indent << Align_("dst.mipLevel:") << *span.DstSubresource.MipLevel << Eol
                    << Indent << Align_("dst.baseLayer:") << *span.DstSubresource.BaseLayer << Eol
                    << Indent << Align_("dst.layerCount:") << span.DstSubresource.LayerCount << Eol
                    << Indent << Align_("dst.offset:") << span.DstOffset << Eol
                    << Indent << Align_("extent:") << span.Extent << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanFillBufferTask& task) const {
    Out << Indent << Align_("dstBuffer:") << Fmt::Quoted(task.DstBuffer->DebugName(), '"') << Eol
        << Indent << Align_("dstOffset:") << Fmt::Offset(task.DstOffset) << Eol
        << Indent << Align_("size:") << Fmt::SizeInBytes(task.Size) << Eol
        << Indent << Align_("pattern:") << Fmt::Offset(task.Pattern) << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanClearColorImageTask& task) const {
    Out << Indent << Align_("dstImage:") << Fmt::Quoted(task.DstImage->DebugName(), '"') << Eol
        << Indent << Align_("dstLayout:") << task.DstLayout << Eol
        << Indent << Align_("ranges:") << '[' << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Ranges.size());
        for (const auto& span : task.Ranges) {
            Out << Indent << "Range {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << Align_("aspectMask:") << span.AspectMask << Eol
                    << Indent << Align_("mipLevel:") << *span.BaseMipLevel << Eol
                    << Indent << Align_("levelCount:") << span.LevelCount << Eol
                    << Indent << Align_("baseLayer:") << *span.BaseLayer << Eol
                    << Indent << Align_("layerCount:") << span.LayerCount << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanClearDepthStencilImageTask& task) const {
    Out << Indent << Align_("dstImage:") << Fmt::Quoted(task.DstImage->DebugName(), '"') << Eol
        << Indent << Align_("dstLayout:") << task.DstLayout << Eol
        << Indent << Align_("clearValue.depth:") << task.ClearValue.depth << Eol
        << Indent << Align_("clearValue.stencil:") << task.ClearValue.stencil << Eol
        << Indent << Align_("ranges:") << '[' << Eol;
    {
        const Fmt::FIndent::FScope indentScope{ Indent };
        auto sep = Fmt::NotLastTime(',', task.Ranges.size());
        for (const auto& span : task.Ranges) {
            Out << Indent << "Range {" << Eol;
            {
                const Fmt::FIndent::FScope subIndent{ Indent };
                Out << Indent << Align_("aspectMask:") << span.AspectMask << Eol
                    << Indent << Align_("mipLevel:") << *span.BaseMipLevel << Eol
                    << Indent << Align_("levelCount:") << span.LevelCount << Eol
                    << Indent << Align_("baseLayer:") << *span.BaseLayer << Eol
                    << Indent << Align_("layerCount:") << span.LayerCount << Eol;
            }
            Out << Indent << "}" << sep << Eol;
        }
    }
    Out << Indent << "]" << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanUpdateBufferTask& task) const {
    Out << Indent << Align_("dstBuffer:") << Fmt::Quoted(task.DstBuffer->DebugName(), '"') << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanPresentTask& task) const {
    Out << Indent << Align_("srcImage:") << Fmt::Quoted(task.SrcImage->DebugName(), '"') << Eol
        << Indent << Align_("layer:") << *task.Layer << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanBuildRayTracingGeometryTask& task) const {
    Out << Indent << Align_("geometry:") << task.RTGeometry->DebugName() << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanBuildRayTracingSceneTask& task) const {
    Out << Indent << Align_("scene:") << task.RTScene->DebugName() << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanUpdateRayTracingShaderTableTask& task) const {
    Out << Indent << Align_("scene:") << task.RTScene->DebugName() << Eol
        << Indent << Align_("shaderTable:") << task.ShaderTable->DebugName() << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanTraceRaysTask& task) const {
    Out << Indent << Align_("groupCount:") << task.GroupCount << Eol
        << Indent << Align_("shaderTable:") << task.ShaderTable->DebugName() << Eol;
}
//----------------------------------------------------------------------------
inline void FVulkanLocalDebugger::FRawTextDump_::DumpTaskInfo(const FVulkanGenerateMipmapsTask& task) const {
    Out << Indent << Align_("image:") << task.Image->DebugName() << Eol
        << Indent << Align_("baseLayer:") << task.BaseLayer << Eol
        << Indent << Align_("layerCount:") << task.LayerCount << Eol
        << Indent << Align_("baseMipLevel:") << task.BaseMipLevel << Eol
        << Indent << Align_("levelCount:") << task.LevelCount << Eol;
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
