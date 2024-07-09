// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Pipeline/VulkanPipelineCompiler.h"

#include "Vulkan/Instance/VulkanInstance.h"
#include "Vulkan/Pipeline/VulkanDebuggableShaderData.h"
#include "Vulkan/Pipeline/VulkanSpirvCompiler.h"

#if USE_PPE_RHIDEBUG
#   include "RHI/EnumToString.h"
#endif

#include "Diagnostic/Logger.h"
#include "Meta/Functor.h"
#include "IO/Format.h"
#include "Memory/SharedBuffer.h"

namespace PPE {
namespace RHI {
LOG_CATEGORY(, VulkanPipelineCompiler)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
NODISCARD static IPipelineCompiler::FLogger VulkanPipelineCompilerLogger_() {
#if USE_PPE_LOGGER
    return [](
        ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringView& text,
        Opaq::object_init&& object) {
        FLogger::LogStructured(
            LogCategory_VulkanPipelineCompiler(),
            FLoggerSiteInfo{verbosity, source, line},
            text, std::move(object));
    };
#else
    return NoFunction;
#endif
}
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

    dstAccess->Flags = dstAccess->Flags | srcAccess.Flags;
    dstAccess->ShaderStages = dstAccess->ShaderStages | srcAccess.ShaderStages;

    if (srcAccess.MemoryAccess != EResourceAccess::Unknown) {
        Assert(dstAccess->MemoryAccess == EResourceAccess::Unknown ||
            dstAccess->MemoryAccess == srcAccess.MemoryAccess);
        dstAccess->MemoryAccess = srcAccess.MemoryAccess;
    }

    if ((dstAccess->Flags & (EResourceFlags::InvalidateBefore | EResourceFlags::Read)) &&
        (dstAccess->MemoryAccess == EResourceAccess::ShaderStorage)) {
        dstAccess->Flags = dstAccess->Flags - EResourceFlags::InvalidateBefore;
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
                    rhs->State |= EResourceShaderStages_FromShaders(dst->StageFlags);
                    return true;
                }
            }
            return false;
        },
        [&](const FPipelineDesc::FSampler& ) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FSampler>(&dst->Data)) {
                if (Ensure(src.Index == dst->Index)) {
                    Unused(rhs);
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
                    rhs->State |= EResourceShaderStages_FromShaders(dst->StageFlags);
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
                    rhs->State |= EResourceShaderStages_FromShaders(dst->StageFlags);
                    return true;
                }
            }
            return false;
        },
        [&](const FPipelineDesc::FUniformBuffer& lhs) NOEXCEPT -> bool {
            if (auto* const rhs = std::get_if<FPipelineDesc::FUniformBuffer>(&dst->Data)) {
                if (Ensure(lhs.Size == rhs->Size and src.Index == dst->Index)) {
                    dst->StageFlags |= src.StageFlags;
                    rhs->State |= EResourceShaderStages_FromShaders(dst->StageFlags);
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
                    rhs->State |= EResourceShaderStages_FromShaders(dst->StageFlags);
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
                    dst->Emplace_Overwrite(std::move(id), un.second);
                    break;
                }
            }

            Unused(found);
            PPE_CLOG(not found, RHI, Error, "failed to find an unused unique uniform name");
        }
        else {
            PPE_LOG_CHECK(PipelineCompiler, MergeUniformData_(&it->second, un.second));
        }
    }

    return true;
}
//----------------------------------------------------------------------------
NODISCARD static bool MergePipelineResources_(
    FPipelineDesc::FPipelineLayout* dst,
    const FPipelineDesc::FPipelineLayout& src,
    EShaderCompilationFlags flags ) {
    Assert(dst);
	Unused(flags);

    // merge descriptor sets
    for (const auto& srcDs : src.DescriptorSets) {
        bool found = false;

        for (const auto& dstDs : dst->DescriptorSets) {
            if (srcDs.Id == dstDs.Id) {
                Assert(dstDs.Uniforms);
                Assert(srcDs.Uniforms);

                PPE_LOG_CHECK(PipelineCompiler, srcDs.BindingIndex == dstDs.BindingIndex);
                PPE_LOG_CHECK(PipelineCompiler, MergeUniformMap_(dstDs.Uniforms.get(), *srcDs.Uniforms));

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
        // Vulkan valid user:
        //  * offset must be a multiple of 4
        //  * size must be a multiple of 4
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VUID-vkCmdPushConstants-offset-00368

        PPE_LOG_CHECK(PipelineCompiler, Meta::IsAlignedPow2(4, srcPc.second.Offset));
        PPE_LOG_CHECK(PipelineCompiler, Meta::IsAlignedPow2(4, srcPc.second.Size));

        const auto it = dst->PushConstants.find(srcPc.first);
        if (dst->PushConstants.end() == it) {
            // check intersections
            for (auto& dstPc : dst->PushConstants) {
                // Vulkan valid usage:
                //  * Any two elements of pPushConstantRanges must not include the same stage in stageFlags
                // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VUID-VkPipelineLayoutCreateInfo-pPushConstantRanges-00292

                if (FRange32u::Overlaps(srcPc.second.Offset, srcPc.second.Offset + srcPc.second.Size,
                                        dstPc.second.Offset, dstPc.second.Offset + dstPc.second.Size )) {
                    PPE_CLOG(not (flags & EShaderCompilationFlags::Quiet), PipelineCompiler, Error, ""
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
        PPE_LOG_CHECK(PipelineCompiler, it->second.Offset % 4 == 0);
        PPE_LOG_CHECK(PipelineCompiler, it->second.Size % 4 == 0);
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
                ubuf and ubuf->State & EResourceFlags::BufferDynamicOffset ) {
                sorted.Push(&un.second);
            }
            else
            if (auto* const sbuf = std::get_if<FPipelineDesc::FStorageBuffer>(&un.second.Data);
                sbuf and sbuf->State & EResourceFlags::BufferDynamicOffset ) {
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
            PPE_LOG_CHECK(PipelineCompiler, not bindingIndices.Insert_ReturnIfExists(bindingIndex));
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
    Assert(lhs and lhs->Data());
    Assert(rhs and rhs->Data());

    if (lhs == rhs || lhs->Fingerprint() == rhs->Fingerprint()) {
        // Assert_NoAssume(lhs->Data()->Equals(*rhs->Data())); // #TODO: GlslangToSpv() output is different for 2 consecutive builds :/ (check InsertTraceRecording)
        return true;
    } else {
        Assert_NoAssume(not lhs->Data()->Equals(*rhs->Data()));
        return false;
    }
}
//----------------------------------------------------------------------------
hash_t FVulkanPipelineCompiler::FShaderBinaryDataTraits::operator ()(const PShaderBinaryData& shaderData) const NOEXCEPT {
    Assert(shaderData);
    return hash_value(shaderData->Fingerprint());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanPipelineCompiler::FVulkanPipelineCompiler(Meta::FForceInit)
:   _defaultLogger(VulkanPipelineCompilerLogger_()) {
    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->SpirvCompiler.create<FVulkanSpirvCompiler>(
        exclusiveData->Directories );

    exclusiveData->SpirvCompiler->SetShaderClockFeatures(true, true);
    exclusiveData->SpirvCompiler->SetShaderFeatures(true, true);
}
//----------------------------------------------------------------------------
FVulkanPipelineCompiler::FVulkanPipelineCompiler(const FVulkanDevice& device)
:   _device(device)
,   _defaultLogger(VulkanPipelineCompilerLogger_()) {
    Assert(_device->vkInstance());
    Assert(_device->vkPhysicalDevice());
    Assert(_device->vkDevice());

    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->SpirvCompiler.create<FVulkanSpirvCompiler>(
        exclusiveData->Directories );

    exclusiveData->SpirvCompiler->SetShaderClockFeatures(true, true);
    exclusiveData->SpirvCompiler->SetShaderFeatures(true, true);

#ifdef VK_KHR_shader_clock
    if (_device->Enabled().ShaderClock) {
        VkPhysicalDeviceShaderClockFeaturesKHR clockFeatures{};
        clockFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CLOCK_FEATURES_KHR;

        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &clockFeatures;

        _device->vkGetPhysicalDeviceFeatures2(_device->vkPhysicalDevice(), &deviceFeatures2);

        exclusiveData->SpirvCompiler->SetShaderClockFeatures(
            !!clockFeatures.shaderSubgroupClock,
            !!clockFeatures.shaderDeviceClock );

        exclusiveData->SpirvCompiler->SetShaderFeatures(
            !!deviceFeatures2.features.vertexPipelineStoresAndAtomics,
            !!deviceFeatures2.features.fragmentStoresAndAtomics);
    }
    else
#endif
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        _device->vkGetPhysicalDeviceFeatures(_device->vkPhysicalDevice(), &deviceFeatures);

        exclusiveData->SpirvCompiler->SetShaderFeatures(
            !!deviceFeatures.vertexPipelineStoresAndAtomics,
            !!deviceFeatures.fragmentStoresAndAtomics);
    }
}
//----------------------------------------------------------------------------
FVulkanPipelineCompiler::~FVulkanPipelineCompiler() {
    ReleaseShaderCache();
}
//----------------------------------------------------------------------------
EShaderCompilationFlags FVulkanPipelineCompiler::CompilationFlags() const NOEXCEPT {
    return _data.LockShared()->CompilationFlags;
}
//----------------------------------------------------------------------------
void FVulkanPipelineCompiler::SetCompilationFlags(EShaderCompilationFlags flags) NOEXCEPT {
    const auto exclusiveData = _data.LockExclusive();

    exclusiveData->CompilationFlags = flags;

    if (flags & EShaderCompilationFlags::UseCurrentDeviceLimits and !!_device) {
        Assert(_device->vkInstance());
        exclusiveData->SpirvCompiler->SetCurrentResourceLimits(*_device);
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
    if (nullptr == _device)
        return; // not device specific -> can't prune unused variants

    const auto exclusiveData = _data.LockExclusive();

    for (auto it = exclusiveData->ShaderCache.begin(); it != exclusiveData->ShaderCache.end(); ) {
        if (it->second->RefCount() > 1) {
            ++it;
            continue;
        }

        it->second.as<FVulkanDebuggableShaderModule>()->TearDown(*_device);

        it = exclusiveData->ShaderCache.erase_ReturnNext(it);
    }
}
//----------------------------------------------------------------------------
void FVulkanPipelineCompiler::ReleaseShaderCache() {
    if (nullptr == _device)
        return; // not device specific -> can't prune unused variants

    const auto exclusiveData = _data.LockExclusive();

    for (const auto& it : exclusiveData->ShaderCache) {
        Assert_NoAssume(it.second->RefCount() == 1);
        it.second.as<FVulkanDebuggableShaderModule>()->TearDown(*_device);
    }

    exclusiveData->ShaderCache.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
// IsSupported()
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::IsSupported(const FMeshPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT {
    if (desc.Shaders.empty())
        return false;

    if (not IsDstFormatSupported_(fmt, !!_device))
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

    if (not IsDstFormatSupported_(fmt, !!_device))
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

    if (not IsDstFormatSupported_(fmt, !!_device))
        return false;

    for (const auto& it : desc.Shaders) {
        if (not IsSupported_(it.second.Data))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::IsSupported(const FComputePipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT {
    if (not IsDstFormatSupported_(fmt, !!_device))
        return false;

    return IsSupported_(desc.Shader.Data);
}
//----------------------------------------------------------------------------
// Compile
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::Compile(FMeshPipelineDesc& desc, EShaderLangFormat fmt, const FLogger& logger) {
    Assert_NoAssume(IsSupported(desc, fmt));

    const auto exclusiveData = _data.LockExclusive();
    const bool createModule = (Meta::EnumAnd(fmt, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::ShaderModule);
    const EShaderLangFormat spirvFormat = (not createModule ? fmt : (fmt - EShaderLangFormat::_StorageFormatMask) | EShaderLangFormat::SPIRV);

    FMeshPipelineDesc ppln;

    for (const auto& sh : desc.Shaders) {
        Assert(not sh.second.Data.empty());

        const auto it = HighestPriorityShaderFormat_(sh.second.Data, spirvFormat);
        if (sh.second.Data.end() == it) {
            logger(ELoggerVerbosity::Error, "@unknown", 0, "no suitable shader format found!"_view, {
                {"Pipeline", "Mesh"},
                {"ShaderType", Meta::EnumOrd(sh.first)},
                {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
            });
            return false;
        }

        // compile GLSL
        if (auto* const pShaderSourceRef = std::get_if<PShaderSource>(&it->second)) {
            FMeshPipelineDesc::FShader shader;
            FVulkanSpirvCompiler::FShaderReflection reflection;

            FConstChar sourceFile = "@unknown";
#if USE_PPE_RHIDEBUG
            sourceFile = (*pShaderSourceRef)->DebugName();
#endif

            FSharedBuffer content{ (*pShaderSourceRef)->Data()->LoadShaderSource() };
            if (not content) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to load shader source!"_view, {
                    {"Pipeline", "Mesh"},
                    {"ShaderType", Meta::EnumOrd(sh.first)},
                    {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
                });
                return false;
            }
            content.Materialize();

            if (not exclusiveData->SpirvCompiler->Compile(
                &shader, &reflection, logger,
                sh.first, it->first, spirvFormat,
                (*pShaderSourceRef)->EntryPoint(),
                content, sourceFile )) {
                return false;
            }

            if (createModule && not CreateVulkanShader_(&shader, exclusiveData->ShaderCache, logger)) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to create vulkan shader module!"_view, {
                    {"Pipeline", "Mesh"},
                    {"ShaderType", Meta::EnumOrd(sh.first)},
                    {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
                });
                return false;
            }

            if (not MergePipelineResources_(&ppln.PipelineLayout, reflection.Layout, exclusiveData->CompilationFlags)) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to merge vulkan pipeline layout!"_view, {
                    {"Pipeline", "Mesh"},
                    {"ShaderType", Meta::EnumOrd(sh.first)},
                    {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
                });
                return false;
            }

            switch (sh.first) {
            case EShaderType::MeshTask:
                ppln.DefaultTaskGroupSize = reflection.Mesh.TaskGroupSize;
                ppln.TaskSizeSpecialization = reflection.Mesh.TaskGroupSpecialization;
                break;
            case EShaderType::Mesh:
                ppln.MaxIndices = reflection.Mesh.MaxIndices;
                ppln.MaxVertices = reflection.Mesh.MaxVertices;
                ppln.Topology = reflection.Mesh.Topology;
                ppln.DefaultMeshGroupSize = reflection.Mesh.MeshGroupSize;
                ppln.MeshSizeSpecialization = reflection.Mesh.MeshGroupSpecialization;
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
            logger(ELoggerVerbosity::Error, "@unknown", 0, "invalid shader data type, expected a source!"_view, {
                {"Pipeline", "Mesh"},
                {"ShaderType", Meta::EnumOrd(sh.first)},
                {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
                {"ShaderDataType", it->second.index()},
            });
            return false;
        }
    }

    UpdateBufferDynamicOffsets_(&ppln.PipelineLayout.DescriptorSets);

    desc = std::move(ppln);

    ONLY_IF_RHIDEBUG(AssertRelease_NoAssume(CheckDescriptorBindings_(desc)));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::Compile(FRayTracingPipelineDesc& desc, EShaderLangFormat fmt, const FLogger& logger) {
    Assert_NoAssume(IsSupported(desc, fmt));

    const auto exclusiveData = _data.LockExclusive();
    const bool createModule = (Meta::EnumAnd(fmt, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::ShaderModule);
    const EShaderLangFormat spirvFormat = (not createModule ? fmt : (fmt - EShaderLangFormat::_StorageFormatMask) | EShaderLangFormat::SPIRV);

    FRayTracingPipelineDesc ppln;

    for (const auto& sh : desc.Shaders) {
        Assert(not sh.second.Data.empty());

        const auto it = HighestPriorityShaderFormat_(sh.second.Data, spirvFormat);
        if (sh.second.Data.end() == it) {
            logger(ELoggerVerbosity::Error, "@unknown", 0, "no suitable shader format found!"_view, {
                {"Pipeline", "RayTracing"},
                {"ShaderName", sh.first.MakeView()},
                {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
            });
            return false;
        }

        // compile GLSL
        if (auto* const pShaderSourceRef = std::get_if<PShaderSource>(&it->second)) {
            FConstChar sourceFile = "@unknown";
#if USE_PPE_RHIDEBUG
            sourceFile = (*pShaderSourceRef)->DebugName();
#endif

            FSharedBuffer content{ (*pShaderSourceRef)->Data()->LoadShaderSource() };
            if (not content) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to load shader source!"_view, {
                    {"Pipeline", "RayTracing"},
                    {"ShaderName", sh.first.MakeView()},
                    {"ShaderType", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
                });
                return false;
            }
            content.Materialize();

            FRayTracingPipelineDesc::FRTShader shader;
            FVulkanSpirvCompiler::FShaderReflection reflection;
            if (not exclusiveData->SpirvCompiler->Compile(
                &shader, &reflection, logger,
                sh.second.Type, it->first, spirvFormat,
                (*pShaderSourceRef)->EntryPoint(),
                content, sourceFile)) {
                return false;
            }

            if (createModule && not CreateVulkanShader_(&shader, exclusiveData->ShaderCache, logger)) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to create vulkan shader module!"_view, {
                    {"Pipeline", "RayTracing"},
                    {"ShaderName", sh.first.MakeView()},
                    {"ShaderType", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
                });
                return false;
            }

            if (not MergePipelineResources_(&ppln.PipelineLayout, reflection.Layout, exclusiveData->CompilationFlags)) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to merge vulkan pipeline layout!"_view, {
                    {"Pipeline", "RayTracing"},
                    {"ShaderName", sh.first.MakeView()},
                    {"ShaderType", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
                });
                return false;
            }

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
            logger(ELoggerVerbosity::Error, "@unknown", 0, "invalid shader data type, expected a source!"_view, {
                {"Pipeline", "RayTracing"},
                {"ShaderName", sh.first.MakeView()},
                {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
                {"ShaderDataType", it->second.index()},
            });
            return false;
        }
    }

    UpdateBufferDynamicOffsets_(&ppln.PipelineLayout.DescriptorSets);

    desc = std::move(ppln);

    ONLY_IF_RHIDEBUG(AssertRelease_NoAssume(CheckDescriptorBindings_(desc)));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::Compile(FGraphicsPipelineDesc& desc, EShaderLangFormat fmt, const FLogger& logger) {
    Assert_NoAssume(IsSupported(desc, fmt));

    const auto exclusiveData = _data.LockExclusive();
    const bool createModule = (Meta::EnumAnd(fmt, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::ShaderModule);
    const EShaderLangFormat spirvFormat = (not createModule ? fmt : (fmt - EShaderLangFormat::_StorageFormatMask) | EShaderLangFormat::SPIRV);

    FGraphicsPipelineDesc ppln;

    for (const auto& sh : desc.Shaders) {
        Assert(not sh.second.Data.empty());

        const auto it = HighestPriorityShaderFormat_(sh.second.Data, spirvFormat);
        if (sh.second.Data.end() == it) {
            logger(ELoggerVerbosity::Error, "@unknown", 0, "no suitable shader format found!"_view, {
                {"Pipeline", "Graphics"},
                {"ShaderType", Meta::EnumOrd(sh.first)},
                {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
            });
            return false;
        }

        // compile GLSL
        if (auto* const pShaderSourceRef = std::get_if<PShaderSource>(&it->second)) {
            FConstChar sourceFile = "@unknown";
#if USE_PPE_RHIDEBUG
            sourceFile = (*pShaderSourceRef)->DebugName();
#endif

            FSharedBuffer content{ (*pShaderSourceRef)->Data()->LoadShaderSource() };
            if (not content) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to load shader source!"_view, {
                    {"Pipeline", "Graphics"},
                    {"ShaderType", Meta::EnumOrd(sh.first)},
                    {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
                });
                return false;
            }
            content.Materialize();

            FGraphicsPipelineDesc::FShader shader;
            FVulkanSpirvCompiler::FShaderReflection reflection;
            if (not exclusiveData->SpirvCompiler->Compile(
                &shader, &reflection, logger,
                sh.first, it->first, spirvFormat,
                (*pShaderSourceRef)->EntryPoint(),
                content, sourceFile)) {
                return false;
            }

            if (createModule && not CreateVulkanShader_(&shader, exclusiveData->ShaderCache, logger)) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to create vulkan shader module!"_view, {
                    {"Pipeline", "Graphics"},
                    {"ShaderType", Meta::EnumOrd(sh.first)},
                    {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
                });
                return false;
            }

            if (not MergePipelineResources_(&ppln.PipelineLayout, reflection.Layout, exclusiveData->CompilationFlags)) {
                logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to merge vulkan pipeline layout!"_view, {
                    {"Pipeline", "Graphics"},
                    {"ShaderType", Meta::EnumOrd(sh.first)},
                    {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                    {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
                });
                return false;
            }

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
            logger(ELoggerVerbosity::Error, "@unknown", 0, "invalid shader data type, expected a source!"_view, {
                {"Pipeline", "Graphics"},
                {"ShaderType", Meta::EnumOrd(sh.first)},
                {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
                {"ShaderDataType", it->second.index()},
            });
            return false;
        }
    }

    ValidatePrimitiveTopology_(&ppln.SupportedTopology);
    UpdateBufferDynamicOffsets_(&ppln.PipelineLayout.DescriptorSets);

    desc = std::move(ppln);

    ONLY_IF_RHIDEBUG(AssertRelease_NoAssume(CheckDescriptorBindings_(desc)));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::Compile(FComputePipelineDesc& desc, EShaderLangFormat fmt, const FLogger& logger) {
    Assert_NoAssume(IsSupported(desc, fmt));

    const auto exclusiveData = _data.LockExclusive();
    const bool createModule = (Meta::EnumAnd(fmt, EShaderLangFormat::_StorageFormatMask) == EShaderLangFormat::ShaderModule);
    const EShaderLangFormat spirvFormat = (not createModule ? fmt : (fmt - EShaderLangFormat::_StorageFormatMask) | EShaderLangFormat::SPIRV);

    FComputePipelineDesc ppln;

    Assert(not desc.Shader.Data.empty());

    const auto it = HighestPriorityShaderFormat_(desc.Shader.Data, spirvFormat);
    if (desc.Shader.Data.cend() == it) {
        logger(ELoggerVerbosity::Error, "@unknown", 0, "no suitable shader format found!"_view, {
            {"Pipeline", "Compute"},
            {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
        });
        return false;
    }

    // compile GLSL
    if (auto* const pShaderSourceRef = std::get_if<PShaderSource>(&it->second)) {
        FConstChar sourceFile = "@unknown";
#if USE_PPE_RHIDEBUG
        sourceFile = (*pShaderSourceRef)->DebugName();
#endif

        FSharedBuffer content{ (*pShaderSourceRef)->Data()->LoadShaderSource() };
        if (not content) {
            logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to load shader source!"_view, {
                {"Pipeline", "Comptue"},
                {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
            });
            return false;
        }
        content.Materialize();

        FVulkanSpirvCompiler::FShaderReflection reflection;
        if (not exclusiveData->SpirvCompiler->Compile(
            &ppln.Shader, &reflection, logger,
            EShaderType::Compute, it->first, spirvFormat,
            (*pShaderSourceRef)->EntryPoint(),
            content, sourceFile )) {
            return false;
        }

        if (createModule && not CreateVulkanShader_(&ppln.Shader, exclusiveData->ShaderCache, logger)) {
            logger(ELoggerVerbosity::Error, sourceFile, 0, "failed to create vulkan shader module!"_view, {
                {"Pipeline", "Comptue"},
                {"ShaderLangFormat", Meta::EnumOrd(it->first)},
                {"SpirvFormat", Meta::EnumOrd(spirvFormat)}
            });
            return false;
        }

        ppln.DefaultLocalGroupSize = reflection.Compute.LocalGroupSize;
        ppln.LocalSizeSpecialization = reflection.Compute.LocalGroupSpecialization;
        ppln.PipelineLayout = std::move(reflection.Layout);
    }
    else {
        logger(ELoggerVerbosity::Error, "@unknown", 0, "invalid shader data type, expected a source!"_view, {
            {"Pipeline", "Compute"},
            {"ShaderLangFormat", Meta::EnumOrd(it->first)},
            {"SpirvFormat", Meta::EnumOrd(spirvFormat)},
            {"ShaderDataType", it->second.index()},
        });
        return false;
    }

    UpdateBufferDynamicOffsets_(&ppln.PipelineLayout.DescriptorSets);

    desc = std::move(ppln);

    ONLY_IF_RHIDEBUG(AssertRelease_NoAssume(CheckDescriptorBindings_(desc)));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineCompiler::CreateVulkanShader_(FPipelineDesc::FShader* shader, FShaderCompiledModuleCache& shaderCache, const FLogger& logger) const {
    Assert(shader);
    Assert(_device);

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
                VK_CHECK(_device->vkCreateShaderModule(
                    _device->vkDevice(),
                    &shaderInfo,
                    _device->vkAllocator(),
                    &shaderId ));
                Assert(VK_NULL_HANDLE != shaderId);

                PShaderModule shaderModule;
                switch (Meta::EnumAnd(sh->first, EShaderLangFormat::_DebugModeMask)) {
#if USE_PPE_RHIDEBUG
                case EShaderLangFormat::EnableDebugTrace:
                case EShaderLangFormat::EnableProfiling:
                case EShaderLangFormat::EnableTimeMap:
                {
                    auto* const debuggableSpirv = checked_cast<const FVulkanDebuggableShaderSPIRV*>(spirvData.get());
                    TRefPtr<FVulkanDebuggableShaderModule> vulkanShaderModule = NEW_REF(PipelineCompiler, FVulkanDebuggableShaderModule, shaderId, *debuggableSpirv);
                    PPE_LOG_CHECK(RHI, vulkanShaderModule->Construct(_device ARGS_IF_RHIDEBUG(debuggableSpirv->DebugName().MakeView(), debuggableSpirv->DebugInfo())));
                    shaderModule = std::move(vulkanShaderModule);
                    break;
                }
#endif
                case EShaderLangFormat::Unknown:
                default:
                {
                    TRefPtr<FVulkanDebuggableShaderModule> vulkanShaderModule = NEW_REF(PipelineCompiler, FVulkanDebuggableShaderModule, shaderId, spirvData);
                    PPE_LOG_CHECK(RHI, vulkanShaderModule->Construct(_device ARGS_IF_RHIDEBUG(spirvData->DebugName().MakeView(), PVulkanSharedDebugUtils{})));
                    shaderModule = std::move(vulkanShaderModule);
                    break;
                }
                }

                it = shaderCache.insert({ spirvData, std::move(shaderModule) }).first;
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
            logger(ELoggerVerbosity::Warning, "@unknown", 0, "removing unknown shader data"_view, {
                {"ShaderLangFormat", Meta::EnumOrd(sh->first)},
                {"ShaderDataType", sh->second.index()}
            });
            sh = shader->Data.Vector().erase(sh);
            break;
        }
    }

    if (Likely(numSpirvData > 0))
        return true;

    logger(ELoggerVerbosity::Error, "@unknown", 0, "did not find any SPIRV shader data!"_view, {});
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
