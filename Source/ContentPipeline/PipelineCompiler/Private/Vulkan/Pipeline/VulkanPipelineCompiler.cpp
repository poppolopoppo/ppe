﻿#include "stdafx.h"

#include "Vulkan/Pipeline/VulkanPipelineCompiler.h"

#include "Vulkan/Instance/VulkanInstance.h"
#include "Vulkan/Pipeline/VulkanDebuggableShaderData.h"
#include "Vulkan/Pipeline/VulkanSpirvCompiler.h"

#include "Meta/Functor.h"
#include "IO/Format.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR bool IsSrcFormatSupported_(EShaderLangFormat src) {
    switch (Meta::EnumOrd(Meta::EnumAnd(src, EShaderLangFormat::_ApiStorageFormatMask))) {
    case Meta::EnumOrd(EShaderLangFormat::OpenGL | EShaderLangFormat::HighLevel):
    case Meta::EnumOrd(EShaderLangFormat::Vulkan | EShaderLangFormat::HighLevel):
        return true;
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
NODISCARD CONSTEXPR bool IsDstFormatSupported_(EShaderLangFormat dst, bool hasDevice) {
    switch (Meta::EnumOrd(Meta::EnumAnd(dst, EShaderLangFormat::_ApiStorageFormatMask))) {
    case Meta::EnumOrd(EShaderLangFormat::Vulkan | EShaderLangFormat::SPIRV): return true;
    case Meta::EnumOrd(EShaderLangFormat::Vulkan | EShaderLangFormat::ShaderModule): return hasDevice;
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
NODISCARD static bool IsSupported_(const FVulkanPipelineCompiler::FShaderDataMap& shaders) {
    Assert(not shaders.empty());

    for (const auto& it : shaders) {
        if (it.second.index() and IsSrcFormatSupported_(it.first))
            return true;
    }

    return false;
}
//----------------------------------------------------------------------------
CONSTEXPR void MergeShaderAccess_(EResourceState* dstAccess, const EResourceState& srcAccess) {
    if (*dstAccess == srcAccess)
        return;

    *dstAccess |= srcAccess;

    if ((*dstAccess & EResourceState::InvalidateBefore) and
        (*dstAccess & EResourceState::ShaderRead)) {
        *dstAccess -= EResourceState::InvalidateBefore;
    }
}
//----------------------------------------------------------------------------
NODISCARD static bool MergeUniformData_(FPipelineDesc::FVariantUniform* dst, const FPipelineDesc::FVariantUniform& src) {
    Assert(dst);
    return Meta::Visit(src.Data,
        [&](const FPipelineDesc::FTexture& lhs) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FTexture>(&dst->Data)) {
                if (Ensure(lhs.Type == rhs->Type and src.Index == dst->Index)) {
                    dst->StageFlags |= src.StageFlags;
                    rhs->State |= EResourceState_FromShaders(dst->StageFlags);
                    return true;
                }
            }
            return false;
        },
        [&](const FPipelineDesc::FSampler& ) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FSampler>(&dst->Data)) {
                if (Ensure(src.Index == dst->Index)) {
                    UNUSED(rhs);
                    dst->StageFlags |= src.StageFlags;
                    return true;
                }
            }
            return false;
        },
        [&](const FPipelineDesc::FSubpassInput& lhs) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FSubpassInput>(&dst->Data)) {
                if (Ensure(lhs.AttachmentIndex == rhs->AttachmentIndex and lhs.IsMultiSample == rhs->IsMultiSample and src.Index == dst->Index)) {
                    dst->StageFlags |= src.StageFlags;
                    rhs->State |= EResourceState_FromShaders(dst->StageFlags);
                    return true;
                }
            }
            return false;
        },
        [&](const FPipelineDesc::FImage& lhs) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FImage>(&dst->Data)) {
                if (Ensure(lhs.Type == rhs->Type and src.Index == dst->Index)) {
                    MergeShaderAccess_(&rhs->State, lhs.State);

                    dst->StageFlags |= src.StageFlags;
                    rhs->State |= EResourceState_FromShaders(dst->StageFlags);
                    return true;
                }
            }
            return false;
        },
        [&](const FPipelineDesc::FUniformBuffer& lhs) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FUniformBuffer>(&dst->Data)) {
                if (Ensure(lhs.Size == rhs->Size and src.Index == dst->Index)) {
                    dst->StageFlags |= src.StageFlags;
                    rhs->State |= EResourceState_FromShaders(dst->StageFlags);
                    return true;
                }
            }
            return false;
        },
        [&](const FPipelineDesc::FStorageBuffer& lhs) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FStorageBuffer>(&dst->Data)) {
                if (Ensure(lhs.StaticSize == rhs->StaticSize and lhs.ArrayStride == rhs->ArrayStride and src.Index == dst->Index)) {
                    MergeShaderAccess_(&rhs->State, lhs.State);

                    dst->StageFlags |= src.StageFlags;
                    rhs->State |= EResourceState_FromShaders(dst->StageFlags);
                    return true;
                }
            }
            return false;
        },
        [&](const FPipelineDesc::FRayTracingScene& lhs) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FRayTracingScene>(&dst->Data)) {
                if (Ensure(lhs.State == rhs->State)) {
                    dst->StageFlags |= src.StageFlags;
                    return true;
                }
            }
            return false;
        },
        [](const std::monostate& ) NOEXCEPT -> bool {
            return false;
        });
}
//----------------------------------------------------------------------------
NODISCARD static bool MergeUniformMap_(
    FPipelineDesc::FUniformMap* dst,
    const FPipelineDesc::FUniformMap& src ) {
    Assert(dst);

    for (const auto& un : src) {
        const auto it = dst->find(un.first);
        if (dst->end() == it) {
            dst->insert(un);
            continue;
        }

        if (un.second.Index.VKBinding() != it->second.Index.VKBinding()) {
            bool found = false;

            forrange(i, 0, 100) {
                char uniqueName[100];
#if USE_PPE_RHIOPTIMIZEIDS
                const size_t len = Format(uniqueName, "un_{0}", i); // #TODO: retrieve a more relevant name?
#else
                const size_t len = Format(uniqueName, "{0}_{1}", un.first.Name, i);
#endif

                FUniformID id{ FStringView(uniqueName, len) };
                if (not dst->Contains(id) and not src.Contains(id)) {
                    found = true;
                    dst->Add_Overwrite({ std::move(id), un.second });
                    break;
                }
            }

            UNUSED(found);
            CLOG(not found, RHI, Error, L"failed to find an unused unique uniform name");
        }
        else {
            LOG_CHECK(PipelineCompiler, MergeUniformData_(&it->second, un.second));
        }
    }

    return true;
}
//----------------------------------------------------------------------------
NODISCARD static bool MergePipelineResources_(
    FPipelineDesc::FPipelineLayout* dst,
    const FPipelineDesc::FPipelineLayout& src) {
    Assert(dst);

    // merge descriptor sets
    for (const auto& srcDs : src.DescriptorSets) {
        bool found = false;

        for (const auto& dstDs : dst->DescriptorSets) {
            if (srcDs.Id == dstDs.Id) {
                Assert(dstDs.Uniforms);
                Assert(srcDs.Uniforms);

                LOG_CHECK(PipelineCompiler, srcDs.BindingIndex == dstDs.BindingIndex);
                LOG_CHECK(PipelineCompiler, MergeUniformMap_(dstDs.Uniforms.get(), *srcDs.Uniforms));

                found = true;
                break;
            }
        }

        if (not found) // add new descriptor set
            dst->DescriptorSets.Push(srcDs);
    }

    // merge push constants
    for (const auto& srcPc : src.PushConstants ) {
        // Vulkan valid user:
        //  * offset must be a multiple of 4
        //  * size must be a multiple of 4
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VUID-vkCmdPushConstants-offset-00368

        LOG_CHECK(PipelineCompiler, Meta::IsAlignedPow2(4, srcPc.second.Offset));
        LOG_CHECK(PipelineCompiler, Meta::IsAlignedPow2(4, srcPc.second.Size));

        const auto it = dst->PushConstants.find(srcPc.first);
        if (dst->PushConstants.end() == it) {
            // check intersections
            for (auto& dstPc : dst->PushConstants) {
                // Vulkan valid usage:
                //  * Any two elements of pPushConstantRanges must not include the same stage in stageFlags
                // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292

                if (FRange32u::Overlaps(srcPc.second.Offset, srcPc.second.Offset + srcPc.second.Size,
                                        dstPc.second.Offset, dstPc.second.Offset + dstPc.second.Size )) {
                    LOG(PipelineCompiler, Error, L""
                        "push constants with different names uses same memory range, which is not handled:\n"
                        "\tA=({0}):{1}..{2}\n"
                        "\tB=({3}):{4}..{5}\n",
                        srcPc.first, srcPc.second.Offset, srcPc.second.Offset + srcPc.second.Offset + srcPc.second.Size,
                        dstPc.first, dstPc.second.Offset, dstPc.second.Offset + dstPc.second.Offset + dstPc.second.Size );
                    return false;
                }
            }

            // add new push constant
            dst->PushConstants.insert(srcPc);
            continue;
        }

        // merge
        it->second.Size = Max(
            checked_cast<u16>(srcPc.second.Offset + srcPc.second.Size),
            checked_cast<u16>(it->second.Offset + it->second.Size) );
        it->second.Offset = Min(srcPc.second.Offset, it->second.Offset);

        it->second.Size -= it->second.Offset;
        it->second.StageFlags |= srcPc.second.StageFlags;

        // same as above
        LOG_CHECK(PipelineCompiler, it->second.Offset % 4 == 0);
        LOG_CHECK(PipelineCompiler, it->second.Size % 4 == 0);
    }

    return true;
}
//----------------------------------------------------------------------------
static void ValidatePrimitiveTopology_(FGraphicsPipelineDesc::FTopologyBits* topology) {
    Assert(topology);

    if (topology->Get(EPrimitiveTopology::Patch)) {
        topology->SetAllFalse();
        topology->SetTrue(EPrimitiveTopology::Patch);
        return;
    }

    if (topology->Get(EPrimitiveTopology::TriangleListAdjacency) or
        topology->Get(EPrimitiveTopology::TriangleStripAdjacency) ) {
        topology->SetAllFalse();
        topology->SetTrue({ EPrimitiveTopology::TriangleListAdjacency, EPrimitiveTopology::TriangleStripAdjacency });
        return;
    }

    if (topology->Get(EPrimitiveTopology::LineListAdjacency) or
        topology->Get(EPrimitiveTopology::LineStripAdjacency) ) {
        topology->SetAllFalse();
        topology->SetTrue({ EPrimitiveTopology::LineListAdjacency, EPrimitiveTopology::LineStripAdjacency });
        return;
    }

    if (topology->AllFalse()) {
        topology->SetTrue({
            EPrimitiveTopology::Point,
            EPrimitiveTopology::LineList,
            EPrimitiveTopology::LineStrip,
            EPrimitiveTopology::TriangleList,
            EPrimitiveTopology::TriangleStrip,
            EPrimitiveTopology::TriangleFan });
        return;
    }
}
//----------------------------------------------------------------------------
static void UpdateBufferDynamicOffsets_(FPipelineDesc::FDescriptorSets* descriptorSets) {
    Assert(descriptorSets);

    TFixedSizeStack<FPipelineDesc::FVariantUniform*, MaxBufferDynamicOffsets> sorted;

    for (FPipelineDesc::FDescriptorSet& dsLayout : *descriptorSets) {
        for (auto& un : *dsLayout.Uniforms) {
            if (auto* const ubuf = std::get_if<FPipelineDesc::FUniformBuffer>(&un.second.Data);
                ubuf and ubuf->State & EResourceState::_BufferDynamicOffset ) {
                sorted.Push(&un.second);
            }
            else
            if (auto* const sbuf = std::get_if<FPipelineDesc::FStorageBuffer>(&un.second.Data);
                sbuf and sbuf->State & EResourceState::_BufferDynamicOffset ) {
                sorted.Push(&un.second);
            }
        }
    }

    std::sort(sorted.begin(), sorted.end(),
        [](const FPipelineDesc::FVariantUniform* a, const FPipelineDesc::FVariantUniform* b) NOEXCEPT -> bool {
            return (a->Index.VKBinding() < b->Index.VKBinding());
        });

    u32 index{ 0 };
    for (FPipelineDesc::FVariantUniform* un : sorted) {
        if (auto* const ubuf = std::get_if<FPipelineDesc::FUniformBuffer>(&un->Data))
            ubuf->DynamicOffsetIndex = index++;
        else
        if (auto* const sbuf = std::get_if<FPipelineDesc::FStorageBuffer>(&un->Data))
            sbuf->DynamicOffsetIndex = index++;
    }

    AssertRelease(index <= MaxBufferDynamicOffsets);
}
//----------------------------------------------------------------------------
static auto HighestPriorityShaderFormat_(const FPipelineDesc::FShaderDataMap& shaders, EShaderLangFormat dst) {
    auto result = shaders.find(dst);
    if (shaders.end() != result)
        return result;

    // limit vulkan max version corresponding to <dst>
    const EShaderLangFormat vulkanVer = (
        Meta::EnumAnd(dst, EShaderLangFormat::_ApiMask) == EShaderLangFormat::Vulkan
            ? Meta::EnumAnd(dst, EShaderLangFormat::_VersionMask)
            : EShaderLangFormat::_VersionMask );

    // search for nearest shader format
    foreachitem(it, shaders) {
        if (not IsSrcFormatSupported_(it->first))
            continue;

        // vulkan has more priority than opengl
        const bool currentIsVulkan = (shaders.end() != result and
            Meta::EnumAnd(result->first, EShaderLangFormat::_ApiMask) == EShaderLangFormat::Vulkan );
        const bool pendingIsVulkan = (Meta::EnumAnd(it->first, EShaderLangFormat::_ApiMask) == EShaderLangFormat::Vulkan );

        const EShaderLangFormat currentVer = (shaders.end() != result ?
            Meta::EnumAnd(result->first, EShaderLangFormat::_VersionMask) : EShaderLangFormat::Unknown );
        const EShaderLangFormat pendingVer = Meta::EnumAnd(it->first, EShaderLangFormat::_VersionMask);

        if (currentIsVulkan) {
            if (not pendingIsVulkan)
                continue;

            // compare vulkan versions
            if (Meta::EnumOrd(pendingVer) > Meta::EnumOrd(currentVer))
                result = it;

            continue;
        }

        if (pendingIsVulkan) {
            if (Meta::EnumOrd(pendingVer) <= Meta::EnumOrd(vulkanVer))
                result = it;

            continue;
        }

        // compare opengl versions
        if (Meta::EnumOrd(pendingVer) > Meta::EnumOrd(currentVer))
            result = it;
    }

    return result;
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
NODISCARD static bool CheckDescriptorBindings_(const FPipelineDesc& desc) {
    TFixedSizeFlatSet<u32, 64> bindingIndices;

    for (const FPipelineDesc::FDescriptorSet& ds : desc.PipelineLayout.DescriptorSets) {
        bindingIndices.clear();

        for (const auto& un : *ds.Uniforms) {
            const auto bindingIndex = un.second.Index.VKBinding();
            // binding index already occupied
            LOG_CHECK(PipelineCompiler, not bindingIndices.Insert_ReturnIfExists(bindingIndex));
        }
    }

    return true;
}
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Custom operators for merging shader binary data in shader cache
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::FShaderBinaryDataTraits::operator ()(const PShaderBinaryData& lhs, const PShaderBinaryData& rhs) const NOEXCEPT {
    if (Likely(!!lhs && !!rhs && lhs->Data() && rhs->Data())) {
        if (lhs == rhs || (lhs->Fingerprint() == rhs->Fingerprint() && lhs->Data()->Equals(*rhs->Data())))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
hash_t FVulkanPipelineCompiler::FShaderBinaryDataTraits::operator ()(const PShaderBinaryData& shaderData) const NOEXCEPT {
    Assert(shaderData);
    return hash_value(shaderData->Fingerprint());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanPipelineCompiler::FVulkanPipelineCompiler(Meta::FForceInit) {
    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->SpirvCompiler.reset<FVulkanSpirvCompiler>(
        exclusiveData->Directories );

    exclusiveData->SpirvCompiler->SetShaderClockFeatures(true, true);
    exclusiveData->SpirvCompiler->SetShaderFeatures(true, true);
}
//----------------------------------------------------------------------------
FVulkanPipelineCompiler::FVulkanPipelineCompiler(const FVulkanDeviceInfo& deviceInfo)
:   _deviceInfo(deviceInfo) {
    Assert(_deviceInfo->vkInstance);
    Assert(_deviceInfo->vkPhysicalDevice);
    Assert(_deviceInfo->vkDevice);
    Assert(_deviceInfo->API.vkCreateShaderModule);
    Assert(_deviceInfo->API.vkDestroyShaderModule);

    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->SpirvCompiler.reset<FVulkanSpirvCompiler>(
        exclusiveData->Directories );

    exclusiveData->SpirvCompiler->SetShaderClockFeatures(true, true);
    exclusiveData->SpirvCompiler->SetShaderFeatures(true, true);

#ifdef VK_KHR_shader_clock
    PFN_vkGetPhysicalDeviceFeatures2KHR vkGetPhysicalDeviceFeatures2KHR_ = bit_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
        _deviceInfo->API.instance_api_->global_api_->vkGetInstanceProcAddr(_deviceInfo->vkInstance, "vkGetPhysicalDeviceFeatures2KHR") );

    if (nullptr == vkGetPhysicalDeviceFeatures2KHR_) {
        vkGetPhysicalDeviceFeatures2KHR_ = bit_cast<PFN_vkGetPhysicalDeviceFeatures2KHR>(
            _deviceInfo->API.instance_api_->global_api_->vkGetInstanceProcAddr(_deviceInfo->vkInstance, "vkGetPhysicalDeviceFeatures2") );
    }

    if (vkGetPhysicalDeviceFeatures2KHR_) {
        VkPhysicalDeviceShaderClockFeaturesKHR clockFeatures{};
        clockFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR;

        VkPhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures.pNext = &clockFeatures;

        vkGetPhysicalDeviceFeatures2KHR_(_deviceInfo->vkPhysicalDevice, &deviceFeatures);

        exclusiveData->SpirvCompiler->SetShaderClockFeatures(
            clockFeatures.shaderSubgroupClock,
            clockFeatures.shaderDeviceClock );
        exclusiveData->SpirvCompiler->SetShaderFeatures(
            deviceFeatures.features.vertexPipelineStoresAndAtomics,
            deviceFeatures.features.fragmentStoresAndAtomics );
    }
    else
#endif
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        _deviceInfo->API.instance_api_->vkGetPhysicalDeviceFeatures(_deviceInfo->vkPhysicalDevice, &deviceFeatures);

        exclusiveData->SpirvCompiler->SetShaderFeatures(
            deviceFeatures.vertexPipelineStoresAndAtomics,
            deviceFeatures.fragmentStoresAndAtomics );
    }
}
//----------------------------------------------------------------------------
FVulkanPipelineCompiler::~FVulkanPipelineCompiler() {
    ReleaseShaderCache();
}
//----------------------------------------------------------------------------
void FVulkanPipelineCompiler::SetCompilationFlags(EVulkanShaderCompilationFlags flags) {
    const auto exclusiveData = _data.LockExclusive();

    exclusiveData->CompilationFlags = flags;

    if (flags & EVulkanShaderCompilationFlags::UseCurrentDeviceLimits and !!_deviceInfo) {
        Assert(_deviceInfo->vkInstance);
        exclusiveData->SpirvCompiler->SetCurrentResourceLimits(*_deviceInfo);
    }
    else {
        exclusiveData->SpirvCompiler->SetDefaultResourceLimits();
    }

    exclusiveData->SpirvCompiler->SetCompilationFlags(flags);
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanPipelineCompiler::SetDebugFlags(EShaderLangFormat flags) {
    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->SpirvCompiler->SetShaderDebugFlags(Meta::EnumAnd(
        flags, EShaderLangFormat::_DebugModeMask ));
}
#endif
//----------------------------------------------------------------------------
void FVulkanPipelineCompiler::AddDirectory(const FDirpath& path) {
    Assert(not path.empty());
    Add_Unique(_data.LockExclusive()->Directories, FDirpath{ path });
}
//----------------------------------------------------------------------------
void FVulkanPipelineCompiler::ReleaseUnusedShaders() {
    if (nullptr == _deviceInfo)
        return; // not device specific -> can't prune unused variants

    const auto exclusiveData = _data.LockExclusive();

    for (auto it = exclusiveData->ShaderCache.begin(); it != exclusiveData->ShaderCache.end(); ) {
        if (it->second->RefCount() > 1) {
            ++it;
            continue;
        }

        it->second.as<FVulkanDebuggableShaderModule>()->TearDown(*_deviceInfo);

        it = exclusiveData->ShaderCache.erase_ReturnNext(it);
    }
}
//----------------------------------------------------------------------------
void FVulkanPipelineCompiler::ReleaseShaderCache() {
    if (nullptr == _deviceInfo)
        return; // not device specific -> can't prune unused variants

    const auto exclusiveData = _data.LockExclusive();

    for (const auto& it : exclusiveData->ShaderCache) {
        AssertMessage(L"", it.second->RefCount() == 1);

        it.second.as<FVulkanDebuggableShaderModule>()->TearDown(*_deviceInfo);
    }

    exclusiveData->ShaderCache.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
// IsSupported()
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::IsSupported(const FMeshPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT {
    if (desc.Shaders.empty())
        return false;

    if (not IsDstFormatSupported_(fmt, !!_deviceInfo))
        return false;

    for (const auto& it : desc.Shaders) {
        if (not IsSupported_(it.second.Data))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::IsSupported(const FRayTracingPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT {
    if (desc.Shaders.empty())
        return false;

    if (not IsDstFormatSupported_(fmt, !!_deviceInfo))
        return false;

    for (const auto& it : desc.Shaders) {
        if (not IsSupported_(it.second.Data))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::IsSupported(const FGraphicsPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT {
    if (desc.Shaders.empty())
        return false;

    if (not IsDstFormatSupported_(fmt, !!_deviceInfo))
        return false;

    for (const auto& it : desc.Shaders) {
        if (not IsSupported_(it.second.Data))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::IsSupported(const FComputePipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT {
    if (not IsDstFormatSupported_(fmt, !!_deviceInfo))
        return false;

    return IsSupported_(desc.Shader.Data);
}
//----------------------------------------------------------------------------
// Compile
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::Compile(FMeshPipelineDesc& desc, EShaderLangFormat fmt) {
    Assert_NoAssume(IsSupported(desc, fmt));

    const auto exclusiveData = _data.LockExclusive();
    const bool createModule = (Meta::EnumAnd(fmt, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::ShaderModule);
    const EShaderLangFormat spirvFormat = (not createModule ? fmt : (fmt - EShaderLangFormat::_StorageFormatMask) | EShaderLangFormat::SPIRV);

    FMeshPipelineDesc ppln;

    for (const auto& sh : desc.Shaders) {
        Assert(not sh.second.Data.empty());

        const auto it = HighestPriorityShaderFormat_(sh.second.Data, spirvFormat);
        if (sh.second.Data.end() == it) {
            LOG(PipelineCompiler, Error, L"mesh pipeline: no suitable shader format found!");
            return false;
        }

        // compile GLSL
        if (auto* const pShaderSourceRef = std::get_if<PShaderSource>(&it->second)) {
            FWStringBuilder log;
            FMeshPipelineDesc::FShader shader;
            FVulkanSpirvCompiler::FShaderReflection reflection;
            if (not exclusiveData->SpirvCompiler->Compile(
                &shader, &reflection, &log,
                sh.first, it->first, spirvFormat,
                (*pShaderSourceRef)->EntryPoint(),
                (*pShaderSourceRef)->Data()->c_str()
                ARGS_IF_RHIDEBUG((*pShaderSourceRef)->DebugName().c_str()) )) {
                LOG_DIRECT(PipelineCompiler, Error, log.Written()); // #TODO: export the log when logger is disabled?
                return false;
            }

            if (createModule)
                LOG_CHECK(PipelineCompiler, CreateVulkanShader_(&shader, exclusiveData->ShaderCache));

            LOG_CHECK(PipelineCompiler, MergePipelineResources_(&ppln.PipelineLayout, reflection.Layout));

            switch (sh.first) {
            case EShaderType::MeshTask:
                ppln.DefaultTaskGroupSize = reflection.Mesh.TaskGroupSize;
                ppln.TaskSizeSpec = reflection.Mesh.TaskGroupSpecialization;
                break;
            case EShaderType::Mesh:
                ppln.MaxIndices = reflection.Mesh.MaxIndices;
                ppln.MaxVertices = reflection.Mesh.MaxVertices;
                ppln.Topology = reflection.Mesh.Topology;
                ppln.DefaultMeshGroupSize = reflection.Mesh.MeshGroupSize;
                ppln.MeshSizeSpec = reflection.Mesh.MeshGroupSpecialization;
                break;
            case EShaderType::Fragment:
                ppln.FragmentOutputs = reflection.Fragment.FragmentOutputs;
                ppln.EarlyFragmentTests = reflection.Fragment.EarlyFragmentTests;
                break;

            default: AssertNotImplemented();
            }

            ppln.Shaders.Insert_Overwrite(EShaderType{ sh.first }, std::move(shader));
        }
        else {
            LOG(PipelineCompiler, Error, L"mesh pipeline: invalid shader data type, expected a source!");
            return false;
        }
    }

    UpdateBufferDynamicOffsets_(&ppln.PipelineLayout.DescriptorSets);

    std::swap(desc, ppln);

    Assert_NoAssume(CheckDescriptorBindings_(desc));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::Compile(FRayTracingPipelineDesc& desc, EShaderLangFormat fmt) {
    Assert_NoAssume(IsSupported(desc, fmt));

    const auto exclusiveData = _data.LockExclusive();
    const bool createModule = (Meta::EnumAnd(fmt, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::ShaderModule);
    const EShaderLangFormat spirvFormat = (not createModule ? fmt : (fmt - EShaderLangFormat::_StorageFormatMask) | EShaderLangFormat::SPIRV);

    FRayTracingPipelineDesc ppln;

    for (const auto& sh : desc.Shaders) {
        Assert(not sh.second.Data.empty());

        const auto it = HighestPriorityShaderFormat_(sh.second.Data, spirvFormat);
        if (sh.second.Data.end() == it) {
            LOG(PipelineCompiler, Error, L"ray-tracing pipeline: no suitable shader format found!");
            return false;
        }

        // compile GLSL
        if (auto* const pShaderSourceRef = std::get_if<PShaderSource>(&it->second)) {
            FWStringBuilder log;
            FRayTracingPipelineDesc::FRTShader shader;
            FVulkanSpirvCompiler::FShaderReflection reflection;
            if (not exclusiveData->SpirvCompiler->Compile(
                &shader, &reflection, &log,
                sh.second.Type, it->first, spirvFormat,
                (*pShaderSourceRef)->EntryPoint(),
                (*pShaderSourceRef)->Data()->c_str()
                ARGS_IF_RHIDEBUG((*pShaderSourceRef)->DebugName().c_str()) )) {
                LOG_DIRECT(PipelineCompiler, Error, log.Written()); // #TODO: export the log when logger is disabled?
                return false;
            }

            if (createModule)
                LOG_CHECK(PipelineCompiler, CreateVulkanShader_(&shader, exclusiveData->ShaderCache));

            LOG_CHECK(PipelineCompiler, MergePipelineResources_(&ppln.PipelineLayout, reflection.Layout));

            switch (sh.second.Type) {
            case EShaderType::RayGen:
            case EShaderType::RayAnyHit:
            case EShaderType::RayClosestHit:
            case EShaderType::RayMiss:
            case EShaderType::RayIntersection:
            case EShaderType::RayCallable:
                break; // #TODO

            default: AssertNotImplemented();
            }

            shader.Type = sh.second.Type;
            ppln.Shaders.Insert_Overwrite(FRTShaderID{ sh.first }, std::move(shader));
        }
        else {
            LOG(PipelineCompiler, Error, L"ray-tracing pipeline: invalid shader data type, expected a source!");
            return false;
        }
    }

    UpdateBufferDynamicOffsets_(&ppln.PipelineLayout.DescriptorSets);

    std::swap(desc, ppln);

    Assert_NoAssume(CheckDescriptorBindings_(desc));
    return true;
}

//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::Compile(FGraphicsPipelineDesc& desc, EShaderLangFormat fmt) {
    Assert_NoAssume(IsSupported(desc, fmt));

    const auto exclusiveData = _data.LockExclusive();
    const bool createModule = (Meta::EnumAnd(fmt, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::ShaderModule);
    const EShaderLangFormat spirvFormat = (not createModule ? fmt : (fmt - EShaderLangFormat::_StorageFormatMask) | EShaderLangFormat::SPIRV);

    FGraphicsPipelineDesc ppln;

    for (const auto& sh : desc.Shaders) {
        Assert(not sh.second.Data.empty());

        const auto it = HighestPriorityShaderFormat_(sh.second.Data, spirvFormat);
        if (sh.second.Data.end() == it) {
            LOG(PipelineCompiler, Error, L"graphics pipeline: no suitable shader format found!");
            return false;
        }

        // compile GLSL
        if (auto* const pShaderSourceRef = std::get_if<PShaderSource>(&it->second)) {
            FWStringBuilder log;
            FGraphicsPipelineDesc::FShader shader;
            FVulkanSpirvCompiler::FShaderReflection reflection;
            if (not exclusiveData->SpirvCompiler->Compile(
                &shader, &reflection, &log,
                sh.first, it->first, spirvFormat,
                (*pShaderSourceRef)->EntryPoint(),
                (*pShaderSourceRef)->Data()->c_str()
                ARGS_IF_RHIDEBUG((*pShaderSourceRef)->DebugName().c_str()) )) {
                LOG_DIRECT(PipelineCompiler, Error, log.Written()); // #TODO: export the log when logger is disabled?
                return false;
            }

            if (createModule)
                LOG_CHECK(PipelineCompiler, CreateVulkanShader_(&shader, exclusiveData->ShaderCache));

            LOG_CHECK(PipelineCompiler, MergePipelineResources_(&ppln.PipelineLayout, reflection.Layout));

            ppln.SupportedTopology |= reflection.Vertex.SupportedTopology;

            switch (sh.first) {
            case EShaderType::Vertex:
                ppln.VertexAttributes = reflection.Vertex.VertexAttributes;
                break;
            case EShaderType::TessControl:
                ppln.PatchControlPoints = reflection.Tesselation.PatchControlPoints;
                break;
            case EShaderType::TessEvaluation:
                break;
            case EShaderType::Geometry:
                break;
            case EShaderType::Fragment:
                ppln.FragmentOutputs = reflection.Fragment.FragmentOutputs;
                ppln.EarlyFragmentTests = reflection.Fragment.EarlyFragmentTests;
                break;

            default: AssertNotImplemented();
            }

            ppln.Shaders.Insert_Overwrite(EShaderType{ sh.first }, std::move(shader));
        }
        else {
            LOG(PipelineCompiler, Error, L"graphics pipeline: invalid shader data type, expected a source!");
            return false;
        }
    }

    ValidatePrimitiveTopology_(&ppln.SupportedTopology);
    UpdateBufferDynamicOffsets_(&ppln.PipelineLayout.DescriptorSets);

    std::swap(desc, ppln);

    Assert_NoAssume(CheckDescriptorBindings_(desc));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::Compile(FComputePipelineDesc& desc, EShaderLangFormat fmt) {
    Assert_NoAssume(IsSupported(desc, fmt));

    const auto exclusiveData = _data.LockExclusive();
    const bool createModule = (Meta::EnumAnd(fmt, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::ShaderModule);
    const EShaderLangFormat spirvFormat = (not createModule ? fmt : (fmt - EShaderLangFormat::_StorageFormatMask) | EShaderLangFormat::SPIRV);

    FComputePipelineDesc ppln;

    Assert(not desc.Shader.Data.empty());

    const auto it = HighestPriorityShaderFormat_(desc.Shader.Data, spirvFormat);
    if (desc.Shader.Data.end() == it) {
        LOG(PipelineCompiler, Error, L"compute pipeline: no suitable shader format found!");
        return false;
    }

    // compile GLSL
    if (auto* const pShaderSourceRef = std::get_if<PShaderSource>(&it->second)) {
        FWStringBuilder log;
        FVulkanSpirvCompiler::FShaderReflection reflection;
        if (not exclusiveData->SpirvCompiler->Compile(
            &ppln.Shader, &reflection, &log,
            EShaderType::Compute, it->first, spirvFormat,
            (*pShaderSourceRef)->EntryPoint(),
            (*pShaderSourceRef)->Data()->c_str()
            ARGS_IF_RHIDEBUG((*pShaderSourceRef)->DebugName().c_str()) )) {
            LOG_DIRECT(PipelineCompiler, Error, log.Written()); // #TODO: export the log when logger is disabled?
            return false;
        }

        if (createModule)
            LOG_CHECK(PipelineCompiler, CreateVulkanShader_(&ppln.Shader, exclusiveData->ShaderCache));

        ppln.DefaultLocalGroupSize = reflection.Compute.LocalGroupSize;
        ppln.LocalSizeSpecialization = reflection.Compute.LocalGroupSpecialization;
        ppln.PipelineLayout = std::move(reflection.Layout);
    }
    else {
        LOG(PipelineCompiler, Error, L"compute pipeline: invalid shader data type, expected a source!");
        return false;
    }

    UpdateBufferDynamicOffsets_(&ppln.PipelineLayout.DescriptorSets);

    std::swap(desc, ppln);

    Assert_NoAssume(CheckDescriptorBindings_(desc));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::CreateVulkanShader_(FPipelineDesc::FShader* shader, FShaderCompiledModuleCache& shaderCache) const {
    Assert(shader);
    Assert(_deviceInfo);
    Assert(_deviceInfo->API.vkCreateShaderModule);
    Assert(_deviceInfo->API.vkDestroyShaderModule);

    u32 numSpirvData = 0;

    for (auto sh = shader->Data.begin(); sh != shader->Data.end(); ) {
        switch (Meta::EnumAnd(sh->first, EShaderLangFormat::_StorageFormatMask)) {
        // create shader module from SPIRV
        case EShaderLangFormat::SPIRV: {
            const PShaderBinaryData& spirvData = std::get<PShaderBinaryData>(sh->second);
            Assert(spirvData);

            const EShaderLangFormat moduleFmt = (
                EShaderLangFormat::ShaderModule |
                EShaderLangFormat::Vulkan |
                Meta::EnumAnd(sh->first, EShaderLangFormat::_VersionModeFlagsMask) );

            auto it = shaderCache.find(spirvData);
            if (shaderCache.end() == it) {
                // shader cache miss
                const auto spirvCode = spirvData->Data()->MakeView().Cast<const u32>();

                VkShaderModuleCreateInfo shaderInfo{};
                shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                shaderInfo.pCode = spirvCode.Pointer();
                shaderInfo.codeSize = spirvCode.SizeInBytes();

                VkShaderModule shaderId = VK_NULL_HANDLE;
                VK_CHECK(_deviceInfo->API.vkCreateShaderModule(
                    _deviceInfo->vkDevice,
                    &shaderInfo,
                    _deviceInfo->pAllocator,
                    &shaderId ));
                Assert(VK_NULL_HANDLE != shaderId);

                auto module = NEW_REF(PipelineCompiler, FVulkanDebuggableShaderModule, shaderId, spirvData);
                it = shaderCache.insert({ spirvData, std::move(module) }).first;
            }

            shader->Data.Erase(sh);
            shader->Data.Insert_AssertUnique(moduleFmt, PShaderModule{ it->second });

            sh = shader->Data.begin();
            numSpirvData++;
            break;
        }
        // keep other shader modules
        case EShaderLangFormat::ShaderModule:
            ++sh;
            break;
        // remove unknown shader data
        default:
            LOG(PipelineCompiler, Verbose, L"removing unknown shader data");
            sh = shader->Data.Vector().erase(sh);
            break;
        }
    }

    if (Likely(numSpirvData > 0))
        return true;

    LOG(PipelineCompiler, Error, L"did not find any SPIRV shader data!");
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE