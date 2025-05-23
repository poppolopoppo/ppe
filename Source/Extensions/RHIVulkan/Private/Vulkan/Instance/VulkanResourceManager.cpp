﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Instance/VulkanResourceManager.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

#include "RHIModule.h"
#include "RHI/PipelineCompiler.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformDialog.h"
#include "HAL/TargetRHI.h"
#include "IO/Format.h"
#include "Modular/ModularDomain.h"

#if USE_PPE_RHIDEBUG
#   include "Meta/TypeInfo.h"
#endif
#if USE_PPE_LOGGER
#   include "IO/FormatHelpers.h"
#   include "RHI/EnumToString.h"
#endif

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
LOG_CATEGORY(, CompileShader)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static TFixedSizeStack<EShaderLangFormat, 16> BuiltinFormats_(const FVulkanDevice& device) {
    const EShaderLangFormat ver = device.vkVersion();

    TFixedSizeStack<EShaderLangFormat, 16> results;

    // at first request external managed shader modules
    results.Push(ver | EShaderLangFormat::ShaderModule);

    if (ver > EShaderLangFormat::Vulkan_110)
        results.Push(EShaderLangFormat::VkShader_110);
    if (ver > EShaderLangFormat::Vulkan_100)
        results.Push(EShaderLangFormat::VkShader_100);

    // at second request shader in binary format
    results.Push(ver | EShaderLangFormat::SPIRV);

    if (ver > EShaderLangFormat::Vulkan_110)
        results.Push(EShaderLangFormat::SPIRV_110);
    if (ver > EShaderLangFormat::Vulkan_100)
        results.Push(EShaderLangFormat::SPIRV_100);

    return results;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks>
static void ReleaseCachedResourcesAndReportLeaks_(FVulkanResourceManager& resources, FVulkanResourceManager::TCache<T, _ChunkSize, _MaxChunks>& cache) {
    Assert_NoAssume(cache.CheckInvariants());

    cache.Clear_IgnoreLeaks([&resources](T* pCached) {
        RHI_TRACE("TearDownCache", Meta::type_info<T>.name, pCached->DebugName(), pCached->RefCount());

#if USE_PPE_RHIDEBUG
        if (not Ensure(pCached->RefCount() == 1)) {
            using resource_type = typename T::value_type;
            PPE_SLOG(RHI, Error, "VulkanResourceManager is leaking a cached resource", {
                { "DebugName", pCached->DebugName().MakeView() },
                { "RefCount", pCached->RefCount() },
                { "Type", Meta::type_info<resource_type>.name.MakeView() },
            });
        }
#endif

        pCached->TearDown_Force(resources);
        return true;
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks>
static void ReleasePooledResourcesAndReportLeaks_(FVulkanResourceManager& resources, FVulkanResourceManager::TPool<T, _ChunkSize, _MaxChunks>& pool) {
    Assert_NoAssume(pool.CheckInvariants());
    Assert_NoAssume(pool.NumLiveBlocks_ForDebug() == 0);

    Unused(resources);
    pool.Clear_AssertCompletelyEmpty();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanResourceManager::FVulkanResourceManager(
    const FVulkanDevice& device,
    size_t maxStagingBufferMemory,
    size_t stagingBufferSize )
:   _device(device)
,   _memoryManager(device)
,   _descriptorManager(device)
,   _submissionCounter(0) {

    _staging.MaxStagingBufferMemory = maxStagingBufferMemory < 1_MiB ? UMax : maxStagingBufferMemory;
    _staging.WritePageSize = stagingBufferSize < 1_KiB ? 0 : stagingBufferSize;
    _staging.ReadPageSize = _staging.WritePageSize;
}
//----------------------------------------------------------------------------
FVulkanResourceManager::~FVulkanResourceManager() = default;
//----------------------------------------------------------------------------
bool FVulkanResourceManager::Construct() {
    if (not _memoryManager.Construct()) {
        PPE_LOG(RHI, Error, "failed to initialize vulkan memory manager");
        return false;
    }
    if (not _descriptorManager.Construct()) {
        PPE_LOG(RHI, Error, "failed to initialize vulkan descriptor manager");
        return false;
    }
    if (not CreateEmptyDescriptorSetLayout_(&_emptyDSLayout)) {
        PPE_LOG(RHI, Error, "failed to create empty descriptor set layout");
        return false;
    }
    if (not CheckHostVisibleMemory_()) {
        PPE_LOG(RHI, Error, "failed to check host visible memory");
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
void FVulkanResourceManager::TearDown() {
    TearDownStagingBuffers_();
    ONLY_IF_RHIDEBUG(TearDownShaderDebuggerResources_());

    // release empty descriptor layout
    VerifyRelease( ReleaseResource(_emptyDSLayout, 2u) );

#if USE_PPE_VULKAN_RESOURCETRACKING
    for (const FResourceHandle& leak : *_resources.PooledResourcesAlive_ForDebug.LockExclusive()) {
        leak.Visit([this](auto resourceId) {
            auto* const pResource = ResourcePool_(resourceId).At(resourceId.Index);
            AssertRelease_NoAssume(pResource && pResource->InstanceID() == resourceId.InstanceID);
            AssertRelease_NoAssume(pResource->RefCount() > 0);

            using resource_type = typename Meta::TDecay<decltype(*pResource)>::value_type;
            PPE_SLOG(RHI, Error, "VulkanResourceManager is leaking a pooled resource", {
                { "DebugName", pResource->DebugName().MakeView() },
                { "RefCount", pResource->RefCount() },
                { "Type", Meta::type_info<resource_type>.name.MakeView() },
            });
        });
    }
#endif

    // release all pools (should all be empty !)
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.ImagePool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.BufferPool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.MemoryObjectPool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.GPipelinePool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.CPipelinePool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.MPipelinePool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.RPipelinePool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.RtGeometryPool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.RtScenePool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.RtShaderTablePool);
    ReleasePooledResourcesAndReportLeaks_(*this, _resources.SwapchainPool);

    // release all cached resources (force tear down)
    ReleaseCachedResourcesAndReportLeaks_(*this, _resources.PplnResourcesCache);
    ReleaseCachedResourcesAndReportLeaks_(*this, _resources.PplnLayoutCache);
    ReleaseCachedResourcesAndReportLeaks_(*this, _resources.DslayoutCache);
    ReleaseCachedResourcesAndReportLeaks_(*this, _resources.FramebufferCache);
    ReleaseCachedResourcesAndReportLeaks_(*this, _resources.RenderPassCache);
    ReleaseCachedResourcesAndReportLeaks_(*this, _resources.SamplerCache);

    // release shader cache
    {
        const auto exclusiveShaderCache = _shaderCache.LockExclusive();

        for (PVulkanShaderModule& sh : *exclusiveShaderCache) {
            Assert(sh);
            Assert_NoAssume(sh->RefCount() == 1);

            sh->TearDown(_device);
        }

        exclusiveShaderCache->clear_ReleaseMemory();
    }

    _descriptorManager.TearDown();
    _memoryManager.TearDown();
}
//----------------------------------------------------------------------------
void FVulkanResourceManager::OnSubmit() {
    _submissionCounter.fetch_add(1, std::memory_order_relaxed);
}
//----------------------------------------------------------------------------
// CreateDescriptorSetLayout
//----------------------------------------------------------------------------
FRawDescriptorSetLayoutID FVulkanResourceManager::CreateDescriptorSetLayout(FPipelineDesc::PUniformMap&& uniforms ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    FRawDescriptorSetLayoutID result;
    TResourceProxy<FVulkanDescriptorSetLayout>* setLayout{ nullptr };
    VerifyRelease( CreateDescriptorSetLayout_(&result, &setLayout, std::move(uniforms) ARGS_IF_RHIDEBUG(debugName)) );
    Assert_NoAssume(result.Valid());
    return result;
}
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CreateEmptyDescriptorSetLayout_(FRawDescriptorSetLayoutID* pId) {
    Assert(pId);

    FVulkanDescriptorSetLayout::FBindings bindings;
    FVulkanDescriptorSetLayout::FOptionalFlags optionalFlags;
    TResourceProxy<FVulkanDescriptorSetLayout> emptyKey{
        &bindings, &optionalFlags, _device,
        NEW_REF(RHIResource, FPipelineDesc::FUniformMap)
    };

    auto it = CreateCachedResource_(pId,
        std::move(emptyKey),
        _device,
        bindings.MakeConstView(),
        optionalFlags.MakeConstView()
        ARGS_IF_RHIDEBUG("EmptyDSLayout"));

    if (Likely(it.first))
        return true;

    PPE_LOG(RHI, Error, "failed to create an empty descriptor set layout");
    return false;
}
//----------------------------------------------------------------------------
// CreatePipelineLayout
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CreatePipelineLayout_(
    FRawPipelineLayoutID* pId,
    const TResourceProxy<FVulkanPipelineLayout>** pPplnLayoutRef,
    const FPipelineDesc::FPipelineLayout& desc
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {

    // init pipeline layout mode
    FDSLayouts dsLayouts;
    for (const FPipelineDesc::FDescriptorSet& ds : desc.DescriptorSets) {
        FRawDescriptorSetLayoutID dsId;
        TResourceProxy<FVulkanDescriptorSetLayout>* dsLayout{ nullptr };
        VerifyRelease( CreateDescriptorSetLayout_(&dsId, &dsLayout,
            FPipelineDesc::PUniformMap{ ds.Uniforms }
            ARGS_IF_RHIDEBUG(debugName)) );

        dsLayouts.Push(dsId, dsLayout);
    }

    return CreatePipelineLayout_(pId, pPplnLayoutRef, desc, dsLayouts ARGS_IF_RHIDEBUG(debugName));
}
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CreatePipelineLayout_(
    FRawPipelineLayoutID* pId,
    const TResourceProxy<FVulkanPipelineLayout>** pPplnLayoutRef,
    const FPipelineDesc::FPipelineLayout& desc,
    const FDSLayouts& dslayouts
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(pId);
    Assert(pPplnLayoutRef);
    Assert(_emptyDSLayout);

    TResourceProxy<FVulkanPipelineLayout> layout{ desc, dslayouts.MakeView() };

    auto& pool = ResourcePool_(*pId);
    const auto it = pool.FindOrAdd(std::move(layout),
        [this ARGS_IF_RHIDEBUG(debugName)](TResourceProxy<FVulkanPipelineLayout>* pLayout, FIndex , bool exist) -> bool {
            if (not exist)
                PPE_LOG_CHECK(RHI, pLayout->Construct(
                    _device,
                    ResourceData(_emptyDSLayout, false/* don't add ref for empty layout */).Read()->Layout
                    ARGS_IF_RHIDEBUG(debugName)) );
            pLayout->AddRef(exist ? 1 : 2);
            return true;
        });

    *pPplnLayoutRef = pool[it.first];

    if (Likely(!!*pPplnLayoutRef)) {
        *pId = FRawPipelineLayoutID{ it.first, (*pPplnLayoutRef)->InstanceID() };
        Assert_NoAssume(pId->Valid());

        if (it.second) {
            // release allocated resources if the ds was already cached
            for (auto& ds : dslayouts)
                ReleaseResource(ds.first);
        }

        return true;
    }

    Assert_NoAssume(not pId->Valid());
    return false;
}
//----------------------------------------------------------------------------
// CreateDescriptorSetLayout
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CreateDescriptorSetLayout_(
    FRawDescriptorSetLayoutID* pId,
    TResourceProxy<FVulkanDescriptorSetLayout>** pDSLayout,
    FPipelineDesc::PUniformMap&& uniforms
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(pId);
    Assert(pDSLayout);
    Assert(uniforms);

    FVulkanDescriptorSetLayout::FBindings bindings;
    FVulkanDescriptorSetLayout::FOptionalFlags optionalFlags;
    TResourceProxy<FVulkanDescriptorSetLayout> emptyKey{
        &bindings, &optionalFlags, _device, std::move(uniforms) };

    *pDSLayout = CreateCachedResource_(pId,
        std::move(emptyKey),
        _device,
        std::move(bindings),
        std::move(optionalFlags)
        ARGS_IF_RHIDEBUG(debugName)).first;

    if (Likely(!!*pDSLayout))
        return true;

    ONLY_IF_ASSERT(PPE_LOG(RHI, Error, "failed to create descriptor set layout for '{0}'", debugName));
    return false;
}
//----------------------------------------------------------------------------
// CompilerShaders
//----------------------------------------------------------------------------
template <typename _Desc>
static bool InteractiveCompileShaders_(
    _Desc& desc,
    IPipelineCompiler& compiler,
    EShaderLangFormat shaderFormat
    ARGS_IF_RHIDEBUG(FStringLiteral pipelineType, FConstChar debugName)) {

    FWStringBuilder text;
    FWString sourceFile;
    u32 sourceLine{ 0 };

    const IPipelineCompiler::FLogger logger =
        [&](ELoggerVerbosity verbosity,
        const FConstChar& source, u32 line,
        const FStringView& textToCopy,
        Opaq::object_init&& object) {
            Unused(verbosity, source, line, textToCopy, object);
#if USE_PPE_LOGGER && USE_PPE_RHIDEBUG
            if (not object.empty()) {
                FLogger::LogStructured(
                    LogCategory_CompileShader(),
                    FLoggerSiteInfo{verbosity, source, line},
                    textToCopy, {
                        {"Name", debugName.MakeView()},
                        {"Pipeline", pipelineType.MakeView()},
                        {"Source", Opaq::array_init{source.MakeView(),line}},
                        {"Data", std::move(object)}
                    });
            }
            else {
                FLogger::LogStructured(
                   LogCategory_CompileShader(),
                   FLoggerSiteInfo{verbosity, source, line},
                   textToCopy, {
                       {"Name", debugName.MakeView()},
                       {"Pipeline", pipelineType.MakeView()},
                       {"Source", Opaq::array_init{source.MakeView(),line}},
                   });
            }
#endif
            if (verbosity > ELoggerVerbosity::Warning && source && sourceFile.empty()) {
                sourceFile = ToWString(source);
                sourceLine = line;
            }

            Format(text, L"{}:{}: {}: {}", source, line, verbosity, textToCopy) << Crlf;
        };

RETRY_COMPILE_SHADERS:
    if (Unlikely(not compiler.Compile(desc, shaderFormat, logger))) {
        FPlatformDialog::EResult result = FPlatformDialog::Ignore;
        if (not (compiler.CompilationFlags() ^ EShaderCompilationFlags::Quiet)) {
            result = FPlatformDialog::FileDialog(*sourceFile, sourceLine, text.Written(),
#if USE_PPE_RHIDEBUG
                StringFormat(L"Shader compilation failed: {:q}", debugName),
#else
                L"Shader compilation failed"_view,
#endif
                FPlatformDialog::kAbortRetryIgnore, FPlatformDialog::Error);
        }

        if (result == FPlatformDialog::Abort)
            PPE_THROW_IT(FVulkanException("shader compilation failed", VK_ERROR_UNKNOWN));

        if (result == FPlatformDialog::Retry) {
#if USE_PPE_LOGGER && USE_PPE_RHIDEBUG
            PPE_SLOG(RHI, Emphasis, "retrying shader compilation", {
                {"Name", debugName.MakeView()},
                {"Pipeline", pipelineType.MakeView()},
            });
#endif
            text.clear();
            goto RETRY_COMPILE_SHADERS;
        }

        Assert_NoAssume(result == FPlatformDialog::Ignore);
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _Desc>
bool FVulkanResourceManager::CompileShaders_(_Desc& desc ARGS_IF_RHIDEBUG(FStringLiteral pipelineType, FConstChar debugName)) {
    const EShaderLangFormat fmtShaderModule = (_device.vkVersion() | EShaderLangFormat::ShaderModule);
    const EShaderLangFormat fmtSPIRV = (_device.vkVersion() | EShaderLangFormat::SPIRV);

    // try to use external compilers
    {
        TFixedSizeStack<SPipelineCompiler, 3> compilers;
        FRHIModule::Get(FModularDomain::Get()).ListCompilers(MakeAppendable(compilers));

        for (const SPipelineCompiler& compiler : compilers) {
            if (compiler->IsSupported(desc, fmtShaderModule))
                return InteractiveCompileShaders_(desc, *compiler, fmtShaderModule ARGS_IF_RHIDEBUG(pipelineType, debugName));

            if (compiler->IsSupported(desc, fmtSPIRV)) {
                if (not InteractiveCompileShaders_(desc, *compiler, fmtSPIRV ARGS_IF_RHIDEBUG(pipelineType, debugName)))
                    return false;

                for (auto& sh : desc.Shaders) {
                    if (sh.second.Data.empty())
                        continue;

                    const auto it = sh.second.Data.find(fmtSPIRV);
                    PPE_LOG_CHECK(RHI, it != sh.second.Data.end());

                    PVulkanShaderModule module;
                    PPE_LOG_CHECK(RHI, CompileShaderSPIRV_(&module, it->second));

                    sh.second.Data.clear();
                    sh.second.Data.insert({ fmtSPIRV, PShaderModule(std::move(module)) });
                }

                return true;
            }
        }
    }

    // check if shaders supported by default compiler
    const auto formats = BuiltinFormats_(_device);
    bool isSupported = true;

    for (auto& sh : desc.Shaders) {
        if (sh.second.Data.empty())
            continue;

        bool found = false;
        for (EShaderLangFormat fmt : formats) {
            const auto it = sh.second.Data.find(fmt);
            if (sh.second.Data.end() == it)
                continue;

            if (fmt & EShaderLangFormat::ShaderModule) {
                auto shaderData = it->second;

                sh.second.Data.clear();
                sh.second.Data.insert({ fmt, std::move(shaderData) });

                found = true;
                break;
            }

            if (fmt & EShaderLangFormat::SPIRV) {
                PVulkanShaderModule module;
                PPE_LOG_CHECK(RHI, CompileShaderSPIRV_(&module, it->second));
                Assert(module);

                sh.second.Data.clear();
                sh.second.Data.insert({ fmt, PShaderModule(std::move(module)) });

                found = true;
                break;
            }
        }

        isSupported &= found;
    }

    PPE_CLOG(not isSupported, RHI, Error, "unsupported shader format");
    return isSupported;
}
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CompileShaders_(FComputePipelineDesc& desc ARGS_IF_RHIDEBUG(FStringLiteral pipelineType, FConstChar debugName)) {
    const EShaderLangFormat fmtShaderModule = (_device.vkVersion() | EShaderLangFormat::ShaderModule);
    const EShaderLangFormat fmtSPIRV = (_device.vkVersion() | EShaderLangFormat::SPIRV);

    // try to use external compilers
    {
        TFixedSizeStack<SPipelineCompiler, 3> compilers;
        FRHIModule::Get(FModularDomain::Get()).ListCompilers(MakeAppendable(compilers));

        for (const SPipelineCompiler& compiler : compilers) {
            if (compiler->IsSupported(desc, fmtShaderModule))
                return InteractiveCompileShaders_(desc, *compiler, fmtShaderModule ARGS_IF_RHIDEBUG(pipelineType, debugName));

            if (compiler->IsSupported(desc, fmtSPIRV)) {
                if (not InteractiveCompileShaders_(desc, *compiler, fmtSPIRV ARGS_IF_RHIDEBUG(pipelineType, debugName)))
                    return false;

                const auto it = desc.Shader.Data.find(fmtSPIRV);
                PPE_LOG_CHECK(RHI, desc.Shader.Data.end() != it);

                PVulkanShaderModule module;
                PPE_LOG_CHECK(RHI, CompileShaderSPIRV_(&module, it->second));

                desc.Shader.Data.clear();
                desc.Shader.Data.insert({ fmtSPIRV, PShaderModule(std::move(module)) });

                return true;
            }
        }
    }

    // check if shaders supported by default compiler
    const auto formats = BuiltinFormats_(_device);

    for (EShaderLangFormat fmt : formats) {
        const auto it = desc.Shader.Data.find(fmt);
        if (desc.Shader.Data.end() == it)
            continue;

        if (fmt & EShaderLangFormat::ShaderModule) {
            auto shaderData = it->second;

            desc.Shader.Data.clear();
            desc.Shader.Data.insert({ fmt, std::move(shaderData) });

            return true;
        }

        if (fmt & EShaderLangFormat::SPIRV) {
            PVulkanShaderModule module;
            if (not CompileShaderSPIRV_(&module, it->second))
                return false;

            Assert(module);
            desc.Shader.Data.clear();
            desc.Shader.Data.insert({ fmt, PShaderModule(std::move(module)) });
            return true;
        }
    }

    PPE_LOG(RHI, Error, "unsupported shader format");
    return false;
}
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CompileShaderSPIRV_(PVulkanShaderModule* pShaderModule, const FShaderDataVariant& spirv) {
    Assert(pShaderModule);
    const auto* const shaderData = std::get_if<PShaderBinaryData>(&spirv);
    if (not shaderData || not *shaderData || not (*shaderData)->Data()) {
        PPE_LOG(RHI, Error, "expected valid binary shader data (SPIRV)");
        return false;
    }

    const IShaderData<FRawData>& rawSpirv = *(*shaderData);

    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = rawSpirv.Data()->SizeInBytes();
    info.pCode = reinterpret_cast<const uint32_t*>(rawSpirv.Data()->Pointer());

    VkShaderModule vkShaderModule = VK_NULL_HANDLE;
    VK_CHECK( _device.vkCreateShaderModule(
        _device.vkDevice(),
        &info,
        _device.vkAllocator(),
        &vkShaderModule) );
    Assert_NoAssume(VK_NULL_HANDLE != vkShaderModule);

    *pShaderModule = NEW_REF(RHIShader, FVulkanShaderModule,
        vkShaderModule,
        rawSpirv.Fingerprint(),
        rawSpirv.EntryPoint().MakeView() );

    PPE_LOG_CHECK(RHI, (*pShaderModule)->Construct(_device ARGS_IF_RHIDEBUG(rawSpirv.DebugName().MakeView())));

    _shaderCache.LockExclusive()->push_back(*pShaderModule);

    return true;
}
//----------------------------------------------------------------------------
// CreatePipeline
//----------------------------------------------------------------------------
template <u32 _Uid, typename _Desc>
bool FVulkanResourceManager::CreateDevicePipeline_(details::TResourceId<_Uid>* pId, _Desc& desc ARGS_IF_RHIDEBUG(FStringLiteral pipelineType, FConstChar debugName)) {
    if (not CompileShaders_(desc ARGS_IF_RHIDEBUG(pipelineType, debugName))) {
        ONLY_IF_RHIDEBUG(Unused(pipelineType));
        RHI_LOG(Error, "failed to compile shaders for {1} pipeline '{0}'", debugName, pipelineType);
        return false;
    }

    FRawPipelineLayoutID layoutId;
    const TResourceProxy<FVulkanPipelineLayout>* pLayout{ nullptr };
    VerifyRelease( CreatePipelineLayout_(&layoutId, &pLayout, desc.PipelineLayout ARGS_IF_RHIDEBUG(debugName)) );

    TPooledResource_<_Uid>* const pPipeline = CreatePooledResource_(pId);
    Assert(pPipeline);

    if (Unlikely(not pPipeline->Construct(desc, layoutId ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct {1} pipeline '{0}'", debugName, pipelineType);
        Verify( ReleaseResource_(ResourcePool_(*pId), pPipeline, pId->Pack(), 0) );
        *pId = Default;
        return false;
    }

    Assert_NoAssume(pId->Valid());
    pPipeline->AddRef();
    return true;
}
//----------------------------------------------------------------------------
FRawMPipelineID FVulkanResourceManager::CreatePipeline(FMeshPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(_device.Enabled().MeshShaderNV);

    FRawMPipelineID pipelineId;
    if (CreateDevicePipeline_(&pipelineId, desc ARGS_IF_RHIDEBUG("Mesh", debugName)))
        return pipelineId;

    return Default;
}
//----------------------------------------------------------------------------
FRawGPipelineID FVulkanResourceManager::CreatePipeline(FGraphicsPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    FRawGPipelineID pipelineId;
    if (CreateDevicePipeline_(&pipelineId, desc ARGS_IF_RHIDEBUG("Graphics", debugName)))
        return pipelineId;

    return Default;
}
//----------------------------------------------------------------------------
FRawCPipelineID FVulkanResourceManager::CreatePipeline(FComputePipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    FRawCPipelineID pipelineId;
    if (CreateDevicePipeline_(&pipelineId, desc ARGS_IF_RHIDEBUG("Compute", debugName)))
        return pipelineId;

    return Default;
}
//----------------------------------------------------------------------------
FRawRTPipelineID FVulkanResourceManager::CreatePipeline(FRayTracingPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    FRawRTPipelineID pipelineId;
    if (CreateDevicePipeline_(&pipelineId, desc ARGS_IF_RHIDEBUG("RayTracing", debugName)))
        return pipelineId;

    return Default;
}
//----------------------------------------------------------------------------
// CreateImage
//----------------------------------------------------------------------------
FRawImageID FVulkanResourceManager::CreateImage(
    const FImageDesc& desc,
    const FMemoryDesc& mem,
    EVulkanQueueFamilyMask queues,
    EResourceState defaultState
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    FRawMemoryID memoryId;
    TResourceProxy<FVulkanMemoryObject>* pMemory{ nullptr };
    if (not CreateMemory_(&memoryId, &pMemory, mem ARGS_IF_RHIDEBUG(debugName))) {
        RHI_LOG(Error, "failed to create memory object for image '{0}'", debugName);
        return Default;
    }
    Assert_NoAssume(memoryId.Valid());

    FRawImageID imageId;
    TResourceProxy<FVulkanImage>* const pImage = CreatePooledResource_(&imageId);
    Assert(pImage);

    if (Unlikely(not pImage->Construct(*this, desc, memoryId, pMemory->Data(), queues, defaultState ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct image '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(imageId), pImage, imageId.Pack(), 0) );
        Verify( ReleaseResource_(ResourcePool_(memoryId), pMemory, memoryId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(imageId.Valid());

    pMemory->AddRef();
    pImage->AddRef();
    return imageId;
}
//----------------------------------------------------------------------------
FRawImageID FVulkanResourceManager::CreateImage(
    const FImageDesc& desc,
    FExternalImage externalImage, FOnReleaseExternalImage&& onRelease,
    TMemoryView<const u32> queueFamilyIndices,
    EResourceState defaultState
    ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(externalImage);
    Assert(onRelease);

    FRawImageID imageId;
    TResourceProxy<FVulkanImage>* const pImage = CreatePooledResource_(&imageId);
    Assert(pImage);

    if (Unlikely(not pImage->Construct(_device, desc, externalImage, std::move(onRelease), queueFamilyIndices, defaultState ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct image from external resource '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(imageId), pImage, imageId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(imageId.Valid());

    pImage->AddRef();
    return imageId;
}
//----------------------------------------------------------------------------
FRawImageID FVulkanResourceManager::CreateImage(
    const FVulkanExternalImageDesc& desc,
    FOnReleaseExternalImage&& onRelease
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    FRawImageID imageId;
    TResourceProxy<FVulkanImage>* const pImage = CreatePooledResource_(&imageId);
    Assert(pImage);

    if (Unlikely(not pImage->Construct(_device, desc, std::move(onRelease) ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct image from vulkan create info '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(imageId), pImage, imageId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(imageId.Valid());

    pImage->AddRef();
    return imageId;
}
//----------------------------------------------------------------------------
// CreateBuffer
//----------------------------------------------------------------------------
FRawBufferID FVulkanResourceManager::CreateBuffer(
    const FBufferDesc& desc,
    const FMemoryDesc& mem,
    EVulkanQueueFamilyMask queues
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    FRawMemoryID memoryId;
    TResourceProxy<FVulkanMemoryObject>* pMemory{ nullptr };
    if (not CreateMemory_(&memoryId, &pMemory, mem ARGS_IF_RHIDEBUG(debugName))) {
        RHI_LOG(Error, "failed to create memory object for buffer '{0}'", debugName);
        return Default;
    }
    Assert_NoAssume(memoryId.Valid());

    FRawBufferID bufferId;
    TResourceProxy<FVulkanBuffer>* const pBuffer = CreatePooledResource_(&bufferId);
    Assert(pBuffer);

    if (Unlikely(not pBuffer->Construct(*this, desc, memoryId, pMemory->Data(), queues ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct buffer '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(bufferId), pBuffer, bufferId.Pack(), 0) );
        Verify( ReleaseResource_(ResourcePool_(memoryId), pMemory, memoryId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(bufferId.Valid());

    pMemory->AddRef();
    pBuffer->AddRef();
    return bufferId;
}
//----------------------------------------------------------------------------
FRawBufferID FVulkanResourceManager::CreateBuffer(
    const FBufferDesc& desc,
    FExternalBuffer externalBuffer, FOnReleaseExternalBuffer&& onRelease,
    TMemoryView<const u32> queueFamilyIndices
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(externalBuffer);
    Assert(onRelease);

    FRawBufferID bufferId;
    TResourceProxy<FVulkanBuffer>* const pBuffer = CreatePooledResource_(&bufferId);
    Assert(pBuffer);

    if (Unlikely(not pBuffer->Construct(_device, desc, externalBuffer, std::move(onRelease), queueFamilyIndices ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct buffer '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(bufferId), pBuffer, bufferId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(bufferId.Valid());

    pBuffer->AddRef();
    return bufferId;
}
//----------------------------------------------------------------------------
FRawBufferID FVulkanResourceManager::CreateBuffer(
    const FVulkanExternalBufferDesc& desc,
    FOnReleaseExternalBuffer&& onRelease
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    FRawBufferID bufferId;
    TResourceProxy<FVulkanBuffer>* const pBuffer = CreatePooledResource_(&bufferId);
    Assert(pBuffer);

    if (Unlikely(not pBuffer->Construct(_device, desc, std::move(onRelease) ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct buffer '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(bufferId), pBuffer, bufferId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(bufferId.Valid());

    pBuffer->AddRef();
    return bufferId;
}
//----------------------------------------------------------------------------
// CreateSampler
//----------------------------------------------------------------------------
FRawSamplerID FVulkanResourceManager::CreateSampler(const FSamplerDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    TResourceProxy<FVulkanSampler> emptyKey{ _device, desc };

    FRawSamplerID samplerId;
    auto it = CreateCachedResource_(&samplerId, std::move(emptyKey), _device ARGS_IF_RHIDEBUG(debugName));
    if (Likely(it.first)) {
        Assert_NoAssume(samplerId.Valid());
        return samplerId;
    }

    RHI_LOG(Error, "failed to create sampler for '{0}", debugName);
    return Default;
}
//----------------------------------------------------------------------------
// CreateRenderPass
//----------------------------------------------------------------------------
FRawRenderPassID FVulkanResourceManager::CreateRenderPass(const TMemoryView<const FVulkanLogicalRenderPass* const>& passes ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    TResourceProxy<FVulkanRenderPass> emptyKey{ passes };

    FRawRenderPassID renderPassId;
    auto it = CreateCachedResource_(&renderPassId, std::move(emptyKey), _device ARGS_IF_RHIDEBUG(debugName));
    if (Likely(it.first)) {
        Assert_NoAssume(renderPassId.Valid());
        return renderPassId;
    }

    RHI_LOG(Error, "failed to create render pass for '{0}", debugName);
    return Default;
}
//----------------------------------------------------------------------------
// CreateFrameBuffer
//----------------------------------------------------------------------------
FRawFramebufferID FVulkanResourceManager::CreateFramebuffer(
    const TMemoryView<const TPair<FRawImageID, FImageViewDesc>>& attachments,
    FRawRenderPassID renderPass,
    const uint2& dim, u32 layers
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(renderPass.Valid());

    TResourceProxy<FVulkanFramebuffer> emptyKey{ attachments, renderPass, dim, layers };

    FRawFramebufferID framebufferId;
    const auto it = CreateCachedResource_(&framebufferId, std::move(emptyKey), *this ARGS_IF_RHIDEBUG(debugName));

    if (Likely(it.first)) {
        Assert_NoAssume(framebufferId.Valid());
        return framebufferId;
    }

    RHI_LOG(Error, "failed to create frame buffer for '{0}", debugName);
    return Default;
}
//----------------------------------------------------------------------------
// CreateDescriptorSet
//----------------------------------------------------------------------------
const FVulkanPipelineResources* FVulkanResourceManager::CreateDescriptorSet(
    const FPipelineResources& desc, FVulkanCommandBatch::FResourceMap& resources ) {
    FRawPipelineResourcesID resourcesId = FPipelineResources::Cached(desc);

    if (Likely(resourcesId)) { // use cached resources ?
        TResourceProxy<FVulkanPipelineResources>* const pPplnResources = ResourcePool_(resourcesId)[resourcesId.Index];

        if (pPplnResources && pPplnResources->InstanceID() == resourcesId.InstanceID) {
            if (resources.insert({ resourcesId.Pack(), 1 }).second)
                pPplnResources->AddRef();

            Assert_NoAssume(pPplnResources->Data().AllResourcesAlive(*this));
            return std::addressof(pPplnResources->Data());
        }
    }

    TResourceProxy<FVulkanDescriptorSetLayout>* const pDSLayout = ResourcePool_(desc.Layout())[desc.Layout().Index];
    PPE_LOG_CHECK(RHI, pDSLayout && pDSLayout->IsCreated() && pDSLayout->InstanceID() == desc.Layout().InstanceID );

    TResourceProxy<FVulkanPipelineResources> emptyKey{ desc };
    const auto [pPplnResources, exist] =
        CreateCachedResource_(&resourcesId, std::move(emptyKey), *this);

    if (Likely(pPplnResources)) {
        Assert_NoAssume(resourcesId.Valid());

        FPipelineResources::SetCached(desc, resourcesId);
        resources.insert({ resourcesId.Pack(), 1 });

        Assert_NoAssume(pPplnResources->Data().AllResourcesAlive(*this));
        return std::addressof(pPplnResources->Data());
    }

    PPE_LOG(RHI, Error, "failed to create descriptor set layout");
    return nullptr;
}
//----------------------------------------------------------------------------
// CacheDescriptorSet
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CacheDescriptorSet(FPipelineResources& desc) {
    FRawPipelineResourcesID resourcesId = FPipelineResources::Cached(desc);
    if (Likely(resourcesId)) {
        TResourceProxy<FVulkanPipelineResources>* const pResources = ResourcePool_(resourcesId)[resourcesId.Index];
        Assert(pResources);

        if (pResources->InstanceID() == resourcesId.InstanceID)
            return true;
    }

    AssertReleaseMessage("dynamic offsets are not supported here", desc.DynamicOffsets().empty());

    TResourceProxy<FVulkanDescriptorSetLayout>* const pDSLayout = ResourcePool_(desc.Layout())[desc.Layout().Index];
    PPE_LOG_CHECK(RHI, pDSLayout->IsCreated() && pDSLayout->InstanceID() == desc.Layout().InstanceID);

    TResourceProxy<FVulkanPipelineResources> emptyKey{ desc };
    const auto [pResources, exist] =
        CreateCachedResource_(&resourcesId, std::move(emptyKey), *this);

    if (Likely(pResources)) {
        Assert_NoAssume(resourcesId.Valid());

        FPipelineResources::SetCached(desc, resourcesId);
        return true;
    }

    PPE_LOG(RHI, Error, "failed to cache descriptor set layout");
    return Default;
}
//----------------------------------------------------------------------------
// ReleaseResource
//----------------------------------------------------------------------------
void FVulkanResourceManager::ReleaseResource(FPipelineResources& desc) {
    if (const FRawPipelineResourcesID resourcesId = FPipelineResources::Cached(desc)) {
        FPipelineResources::SetCached(desc, FRawPipelineResourcesID{});
        ReleaseResource(resourcesId);
    }
}
//----------------------------------------------------------------------------
// CreateRayTracingGeometry
//----------------------------------------------------------------------------
FRawRTGeometryID FVulkanResourceManager::CreateRayTracingGeometry(
    const FRayTracingGeometryDesc& desc,
    const FMemoryDesc& mem
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert_NoAssume(_device.Enabled().RayTracingNV);

    FRawMemoryID memoryId;
    TResourceProxy<FVulkanMemoryObject>* pMemory{ nullptr };
    if (Unlikely(not CreateMemory_(&memoryId, &pMemory, mem ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to create memory object for raytracing geometry for '{0}'", debugName);
        return Default;
    }
    Assert_NoAssume(memoryId.Valid());

    FRawRTGeometryID geometryId;
    TResourceProxy<FVulkanRTGeometry>* const pGeometry = CreatePooledResource_(&geometryId);

    if (Unlikely(not pGeometry->Construct(*this, desc, memoryId, pMemory->Data() ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct raytracing geometry '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(geometryId), pGeometry, geometryId.Pack(), 0) );
        Verify( ReleaseResource_(ResourcePool_(memoryId), pMemory, memoryId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(geometryId.Valid());

    pMemory->AddRef();
    pGeometry->AddRef();
    return geometryId;
}
//----------------------------------------------------------------------------
// CreateRayTracingScene
//----------------------------------------------------------------------------
FRawRTSceneID FVulkanResourceManager::CreateRayTracingScene(
    const FRayTracingSceneDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert_NoAssume(_device.Enabled().RayTracingNV);

    FRawMemoryID memoryId;
    TResourceProxy<FVulkanMemoryObject>* pMemory{ nullptr };
    if (Unlikely(not CreateMemory_(&memoryId, &pMemory, mem ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to create memory object for raytracing scene for '{0}'", debugName);
        return Default;
    }
    Assert_NoAssume(memoryId.Valid());

    FRawRTSceneID sceneId;
    TResourceProxy<FVulkanRTScene>* const pScene = CreatePooledResource_(&sceneId);

    if (Unlikely(not pScene->Construct(*this, desc, memoryId, pMemory->Data() ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct raytracing scene '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(sceneId), pScene, sceneId.Pack(), 0) );
        Verify( ReleaseResource_(ResourcePool_(memoryId), pMemory, memoryId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(sceneId.Valid());

    pMemory->AddRef();
    pScene->AddRef();
    return sceneId;
}
//----------------------------------------------------------------------------
// CreateRayTracingShaderTable
//----------------------------------------------------------------------------
FRawRTShaderTableID FVulkanResourceManager::CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(FConstChar debugName)) {
    FRawRTShaderTableID shaderTableId;
    TResourceProxy<FVulkanRayTracingShaderTable>* const pShaderTable = CreatePooledResource_(&shaderTableId);
    Assert(pShaderTable);

    if (Unlikely(not pShaderTable->Construct(ARG0_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct raytracing shader table '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(shaderTableId), pShaderTable, shaderTableId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(shaderTableId.Valid());

    pShaderTable->AddRef();
    return shaderTableId;
}
//----------------------------------------------------------------------------
// CreateSwapchain
//----------------------------------------------------------------------------
FRawSwapchainID FVulkanResourceManager::CreateSwapchain(
    const FSwapchainDesc& desc, FRawSwapchainID oldSwapchain, FVulkanFrameGraph& fg ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    const FVulkanSwapchain* swapchain = (oldSwapchain ? ResourceDataIFP(oldSwapchain, false, true) : nullptr);
    if (Likely(swapchain)) {
        Assert_NoAssume(oldSwapchain.Valid());

        if (Unlikely(not const_cast<FVulkanSwapchain*>(swapchain)->Construct(fg, desc ARGS_IF_RHIDEBUG(debugName)))) {
            RHI_LOG(Error, "failed to re-construct old swapchain for '{0}'", debugName);
            return Default;
        }

        return oldSwapchain;
    }

    FRawSwapchainID swapchainId;
    TResourceProxy<FVulkanSwapchain>* const pSwapchain = CreatePooledResource_(&swapchainId);
    Assert_NoAssume(pSwapchain);

    if (Unlikely(not pSwapchain->Construct(fg, desc ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct swapchain for '{0}'", debugName);
        VerifyRelease( ReleaseResource_(ResourcePool_(swapchainId), pSwapchain, swapchainId.Pack(), 0) );
        return Default;
    }
    Assert_NoAssume(swapchainId.Valid());

    pSwapchain->AddRef();
    return swapchainId;
}
//----------------------------------------------------------------------------
// RunValidation
//----------------------------------------------------------------------------
void FVulkanResourceManager::RunValidation(u32 maxIteration) {
    const auto garbageCollectIFP = [this](auto* pResource) {
        Assert(pResource);
        if (pResource->IsCreated() and not pResource->Data().AllResourcesAlive(*this)) {
            RHI_TRACE("RunValidation", L"TearDown", Meta::type_info<decltype(*pResource)>.name, pResource->InstanceID(), pResource->RefCount());
            Verify( pResource->RemoveRef(pResource->RefCount()) );
            pResource->TearDown(*this);
            return true; // release the cached item
        }
        else {
            RHI_TRACE("RunValidation", L"KeepAlive", Meta::type_info<decltype(*pResource)>.name, pResource->InstanceID(), pResource->RefCount());
        }
        return false; // keep alive
    };

    _resources.PplnResourcesCache.GarbageCollect(
        _validation.PplnResourcesGC,
        maxIteration,
        [&](TResourceProxy<FVulkanPipelineResources>* pResource) {
            return garbageCollectIFP(pResource);
        });

    _resources.FramebufferCache.GarbageCollect(
        _validation.FrameBuffersGC,
        maxIteration,
        [&](TResourceProxy<FVulkanFramebuffer>* pResource) {
            return garbageCollectIFP(pResource);
        });
}
//----------------------------------------------------------------------------
// ReleaseMemory
//----------------------------------------------------------------------------
void FVulkanResourceManager::ReleaseMemory() {
    // reclaims cached memory from all pools
    _resources.ImagePool.ReleaseCacheMemory();
    _resources.BufferPool.ReleaseCacheMemory();
    _resources.MemoryObjectPool.ReleaseCacheMemory();
    _resources.GPipelinePool.ReleaseCacheMemory();
    _resources.CPipelinePool.ReleaseCacheMemory();
    _resources.MPipelinePool.ReleaseCacheMemory();
    _resources.RPipelinePool.ReleaseCacheMemory();
    _resources.RtGeometryPool.ReleaseCacheMemory();
    _resources.RtScenePool.ReleaseCacheMemory();
    _resources.RtShaderTablePool.ReleaseCacheMemory();
    _resources.SwapchainPool.ReleaseCacheMemory();

    // trim all unused cached resources
    auto trimDownCache = [this](auto& cache, size_t maxIterations) {
        using resource_type = typename Meta::TDecay<decltype(cache)>::value_type;
        std::atomic<size_t> gc;
        cache.GarbageCollect(gc, maxIterations, [this](resource_type* pResource) {
            Assert(pResource);
            RHI_TRACE("TrimDownCache", Meta::type_info<resource_type>.name, pResource->DebugName(), pResource->IsCreated(), pResource->RefCount());
            if (pResource->IsCreated() and pResource->RefCount() == 1) {
                Verify( pResource->RemoveRef(1) );
                pResource->TearDown(*this);
                return true; // release the cached item
            }
            return false; // keep alive
        });
    };

    constexpr size_t maxIterationsForTrim = MaxCached; // else kept alive due to manual AddRef()
    trimDownCache(_resources.PplnResourcesCache, maxIterationsForTrim);
    trimDownCache(_resources.PplnLayoutCache, maxIterationsForTrim);
    trimDownCache(_resources.DslayoutCache, maxIterationsForTrim);
    trimDownCache(_resources.RenderPassCache, maxIterationsForTrim);
    trimDownCache(_resources.FramebufferCache, maxIterationsForTrim);
    trimDownCache(_resources.SamplerCache, maxIterationsForTrim);

    // finally, release memory in allocator
    _memoryManager.ReleaseMemory(*this);
}
//----------------------------------------------------------------------------
// CreateStagingBuffer
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CreateStagingBuffer(FRawBufferID* pId, FStagingBufferIndex* pIndex, EBufferUsage usage) {
    Assert(pId);
    Assert(pIndex);

    EMemoryType memType;
    FStagingBufferPool* pool;
    u32 poolIndexMask;
    ONLY_IF_RHIDEBUG(FConstChar debugName);

    FBufferDesc desc;
    desc.Usage = usage;

    switch (usage) {
    case EBufferUsage::TransferSrc:
        pool = &_staging.Write;
        desc.SetSize(_staging.WritePageSize);
        memType = EMemoryType::HostWrite;
        poolIndexMask = 1u << 30;
        ONLY_IF_RHIDEBUG(debugName = "HostWriteBuffer");
        break;

    case EBufferUsage::TransferDst:
        pool = &_staging.Read;
        desc.SetSize(_staging.ReadPageSize);
        memType = EMemoryType::HostRead;
        poolIndexMask = 2u << 30;
        ONLY_IF_RHIDEBUG(debugName = "HostReadBuffer");
        break;

    case EBufferUsage::Uniform:
        pool = &_staging.Uniform;
        desc.SetSize(_staging.UniformPageSize);
        memType = EMemoryType::HostWrite;
        poolIndexMask = 3u << 30;
        ONLY_IF_RHIDEBUG(debugName = "HostUniformBuffer");
        break;

    default: AssertReleaseFailed("unsupported buffer usage");
    }

    FBufferID* const pBufferId = pool->Allocate([this, &desc, memType ARGS_IF_RHIDEBUG(debugName)](FBufferID* pBufferId) {
        const FRawBufferID rawBufferId = CreateBuffer(
            desc, FMemoryDesc{ memType }, EVulkanQueueFamilyMask::Unknown ARGS_IF_RHIDEBUG(debugName));
        AssertRelease(rawBufferId.Valid());
        INPLACE_NEW(pBufferId, FBufferID){ rawBufferId };
    });

    if (Likely(pBufferId)) {
        Assert_NoAssume(pBufferId->Valid());
        *pId = pBufferId->Id;
        *pIndex = FStagingBufferIndex{ pool->BlockIndex(pBufferId) | poolIndexMask };
        return true;
    }

    ONLY_IF_RHIDEBUG(RHI_LOG(Error, "failed to allocate staging buffer for '{0}'", debugName));
    return false;
}
//----------------------------------------------------------------------------
// ReleaseStagingBuffer
//----------------------------------------------------------------------------
void FVulkanResourceManager::ReleaseStagingBuffer(FStagingBufferIndex index) {
    const u32 blockIndex = (index.Value & ~(3u << 30));

    // keep the staging buffer created for recycling
    switch (index.Value >> 30) {
    case 1: _staging.Write.ReleaseBlock(blockIndex); break;
    case 2: _staging.Read.ReleaseBlock(blockIndex); break;
    case 3: _staging.Uniform.ReleaseBlock(blockIndex); break;
    default: AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
//  TearDownStagingBuffers_
//----------------------------------------------------------------------------
void FVulkanResourceManager::TearDownStagingBuffers_() {
    auto dtor = [this](FBufferID* pBufferId) {
        ReleaseResource(pBufferId->Release());
        Meta::Destroy(pBufferId);
    };

    _staging.Write.Clear_ReleaseMemory(dtor);
    _staging.Read.Clear_ReleaseMemory(dtor);
    _staging.Uniform.Clear_ReleaseMemory(dtor);
}
//----------------------------------------------------------------------------
// CreatePooledResource_
//----------------------------------------------------------------------------
template <u32 _Uid, typename... _Args>
auto FVulkanResourceManager::CreatePooledResource_(details::TResourceId<_Uid>* pId, _Args&&... args) -> TPooledResource_<_Uid>* {
    Assert(pId);

    auto& pool = ResourcePool_(*pId);
    const FIndex index = pool.Allocate();

    TPooledResource_<_Uid>* pResource = pool[index];
    AssertMessage("resource pool overflow", pResource);

    Meta::Construct(pResource, std::forward<_Args>(args)...);
    *pId = details::TResourceId<_Uid>{ index, pResource->InstanceID() };

#if USE_PPE_VULKAN_RESOURCETRACKING
    _resources.TrackPooledResource_ForDebug(pId->Pack());
#endif

    Assert(pId->Valid());
    return pResource;
}
//----------------------------------------------------------------------------
template <u32 _Uid, typename... _Args>
auto FVulkanResourceManager::CreateCachedResource_(
    details::TResourceId<_Uid>* pId,
    TPooledResource_<_Uid>&& rkey,
    _Args&&... args ) -> TPair<TPooledResource_<_Uid>*, bool> {
    Assert(pId);

    auto& pool = ResourcePool_(*pId);
    const TPair<FIndex, bool> it = pool.FindOrAdd(std::move(rkey), [&](TPooledResource_<_Uid>* pResource, FIndex , bool exist) -> bool {
        if (not exist)
            PPE_LOG_CHECK(RHI, pResource->Construct(std::forward<_Args>(args)...));
        pResource->AddRef(exist ? 1 : 2 /* keep in cache */);
        return true;
    });

    if (TPooledResource_<_Uid>* const pResource = pool[it.first]) {
        *pId = details::TResourceId<_Uid>{ it.first, pResource->InstanceID() };
        Assert(pId->Valid());
        return { pResource, it.second };
    }

    Assert(not pId->Valid());
    return { nullptr, false };
}
//----------------------------------------------------------------------------
// CreateMemory_
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CreateMemory_(
    FRawMemoryID* pId,
    TResourceProxy<FVulkanMemoryObject>** pMemory,
    const FMemoryDesc& desc
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(pId);
    Assert(pMemory);

    *pMemory = CreatePooledResource_(pId);
    Assert(*pMemory);

    if (Unlikely(not (*pMemory)->Construct(desc ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, "failed to construct memory object '{0}'", debugName);
        VerifyRelease( ReleaseResource_(ResourcePool_(*pId), *pMemory, pId->Pack(), 0) );
        *pMemory =  nullptr;
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
// CheckHostVisibleMemory_
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CheckHostVisibleMemory_() {
    const auto& props = _device.Capabilities().MemoryProperties;

    VkMemoryRequirements transferMemReq{};
    VkMemoryRequirements uniformMemReq{};
    {
        VkBufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        info.size = 64_KiB;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.pQueueFamilyIndices = nullptr;
        info.queueFamilyIndexCount = 0;

        VkBuffer vkBuffer{ VK_NULL_HANDLE };

        VK_CHECK( _device.vkCreateBuffer(_device.vkDevice(), &info, _device.vkAllocator(), &vkBuffer) );
        _device.vkGetBufferMemoryRequirements(_device.vkDevice(), vkBuffer, &transferMemReq);
        _device.vkDestroyBuffer(_device.vkDevice(), vkBuffer, _device.vkAllocator());

        info.size = 256;
        info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        VK_CHECK( _device.vkCreateBuffer(_device.vkDevice(), &info, _device.vkAllocator(), &vkBuffer) );
        _device.vkGetBufferMemoryRequirements(_device.vkDevice(), vkBuffer, &uniformMemReq);
        _device.vkDestroyBuffer(_device.vkDevice(), vkBuffer, _device.vkAllocator());
    }

    Meta::TStaticBitset<VK_MAX_MEMORY_HEAPS> cachedHeaps;
    Meta::TStaticBitset<VK_MAX_MEMORY_HEAPS> coherentHeaps;
    Meta::TStaticBitset<VK_MAX_MEMORY_HEAPS> uniformHeaps;

    forrange(i, 0, props.memoryTypeCount) {
        const VkMemoryType& memType = props.memoryTypes[i];

        if (transferMemReq.memoryTypeBits & (1u << i)) {
            if (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
                cachedHeaps.set(memType.heapIndex);

            if (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                coherentHeaps.set(memType.heapIndex);
        }

        if ((uniformMemReq.memoryTypeBits & (1u << i)) &&
            (memType.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) ) {
            uniformHeaps.set(memType.heapIndex);
        }
    }

    size_t uniformHeapSize = 0;
    size_t transferHeapSize = 0;
    forrange(i, 0, props.memoryHeapCount) {
        if (uniformHeaps.test(i))
            uniformHeapSize += checked_cast<size_t>(props.memoryHeaps[i].size);
        if (cachedHeaps.test(i) || coherentHeaps.test(i))
            transferHeapSize += checked_cast<size_t>(props.memoryHeaps[i].size);
    }

    _staging.MaxStagingBufferMemory = Min((4 * transferHeapSize) / 5, _staging.MaxStagingBufferMemory);

    size_t uniformSize = (uniformHeapSize / FStagingBufferPool::Capacity);
    size_t transferSize = (_staging.MaxStagingBufferMemory / FStagingBufferPool::Capacity);

#if defined(PLATFORM_PC) || defined(PLATFORM_MOBILE)
    // keep some slack for other processes
    if (uniformSize > 128_MiB)  uniformSize = 32_MiB; else
    if (uniformSize >  64_MiB)  uniformSize = 16_MiB; else
                                uniformSize =  8_MiB;

    if (transferSize > 256_MiB) transferSize = 128_MiB; else
    if (transferSize > 128_MiB) transferSize = 64_MiB; else
                                transferSize = 32_MiB;
#endif

    _staging.UniformPageSize = uniformSize;

    if (_staging.WritePageSize == 0)
        _staging.WritePageSize = _staging.ReadPageSize = transferSize;

    PPE_LOG(RHI, Info, "uniform buffer size: {0}", Fmt::SizeInBytes(_staging.UniformPageSize));
    PPE_LOG(RHI, Info, "write page size: {0}", Fmt::SizeInBytes(_staging.WritePageSize));
    PPE_LOG(RHI, Info, "max staging buffer memory: {0}, max available: {1}", Fmt::SizeInBytes(_staging.MaxStagingBufferMemory), Fmt::SizeInBytes(transferHeapSize));

    return true;
}
//----------------------------------------------------------------------------
// Debugger
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
TPair<FRawDescriptorSetLayoutID, bool> FVulkanResourceManager::CreateDebugDescriptorSetLayout(
    EShaderDebugMode debugMode,
    EShaderStages debuggableShaders ) {
    const u32 key = ((static_cast<u32>(debuggableShaders) & 0xFFFFFFu) | (static_cast<u32>(debugMode) << 24));
    const auto it = _shaderDebug.dsLayoutCaches.find(key);
    if (_shaderDebug.dsLayoutCaches.end() != it)
        return MakePair(it->second, true);

    FPipelineDesc::FStorageBuffer sbDesc;
    sbDesc.DynamicOffsetIndex = 0;
    sbDesc.ArrayStride = sizeof(u32);
    sbDesc.StaticSize = DebugShaderStorageSize(debuggableShaders, debugMode);
    sbDesc.State = EResourceState_ShaderReadWrite |
        EResourceShaderStages_FromShaders(debuggableShaders) |
        EResourceFlags::BufferDynamicOffset;

    FPipelineDesc::FVariantUniform sbUniform;
    sbUniform.ArraySize = 1;
    sbUniform.Data = sbDesc;
    sbUniform.StageFlags = debuggableShaders;
    sbUniform.Index = FBindingIndex{ UMax, 0 };

    FPipelineDesc::PUniformMap uniforms = NEW_REF(RHIResource, FPipelineDesc::FUniformMap);
    uniforms->Emplace_AssertUnique(FUniformID{ "dbg_ShaderTrace" }, std::move(sbUniform));

    const FRawDescriptorSetLayoutID layout = CreateDescriptorSetLayout(std::move(uniforms),
        INLINE_FORMAT(256, "dbg_{0}({1})", debugMode, debuggableShaders));
    Assert_NoAssume(layout.Valid());

    _shaderDebug.dsLayoutCaches.insert_AssertUnique({ key, layout });
    return MakePair(layout, false);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FRawPipelineLayoutID FVulkanResourceManager::CreateDebugPipelineLayout(
    FRawPipelineLayoutID baseLayout,
    EShaderDebugMode debugMode,
    EShaderStages debuggableShaders,
    const FDescriptorSetID& descriptorSet ) {
    Assert(baseLayout.Valid());
    Assert(descriptorSet.Valid());

    FVulkanPipelineLayout const& origin = ResourceData(baseLayout);

    FDSLayouts dsLayouts;
    FPipelineDesc::FPipelineLayout desc;
    auto& dsPool = ResourcePool_(FRawDescriptorSetLayoutID{});

    const auto originReadable = origin.Read();

    // copy push constant ranges
    desc.PushConstants = originReadable->PushConstants;

    // copy descriptor set layouts
    for (const auto& src : originReadable->DescriptorSets) {
        Assert_NoAssume(src.second.Index != DebugDescriptorSet);

        auto& dsLayout = dsPool.Value(src.second.Id.Index);
        dsLayout.AddRef();

        FPipelineDesc::FDescriptorSet dst;
        dst.Id = src.first;
        dst.BindingIndex = src.second.Index;
        dst.Uniforms = dsLayout.Data().Read()->Uniforms;

        desc.DescriptorSets.Push(std::move(dst));
        dsLayouts.Push(src.second.Id, &dsLayout);
    }

    // append descriptor set layout for shader trace
    {
        const auto [dsLayoutId, layoutCacheHit] = CreateDebugDescriptorSetLayout(debugMode, debuggableShaders);
        Assert(dsLayoutId.Valid());

        auto& dsLayout = dsPool.Value(checked_cast<FIndex>(dsLayoutId.Index));
        if (layoutCacheHit)
            dsLayout.AddRef();

        FPipelineDesc::FDescriptorSet dst;
        dst.Id = descriptorSet;
        dst.BindingIndex = DebugDescriptorSet;
        dst.Uniforms = dsLayout.Data().Read()->Uniforms;

        desc.DescriptorSets.Push(std::move(dst));
        dsLayouts.Push(dsLayoutId, &dsLayout);
    }

    FRawPipelineLayoutID pplnLayoutId;
    const TResourceProxy<FVulkanPipelineLayout>* pplnLayout{ nullptr };
    VerifyRelease( CreatePipelineLayout_(&pplnLayoutId, &pplnLayout, desc, dsLayouts,
        (FString("dbg_") + origin.DebugName()).c_str() ));

    return pplnLayoutId;
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanResourceManager::ShaderTimemapPipelines(FShaderTimemapPipelines* pPpln) {
    VerifyRelease( CreateFindMaxValuePipeline1_() );
    VerifyRelease( CreateFindMaxValuePipeline2_() );
    VerifyRelease( CreateTimemapRemapPipeline_() );

    (*pPpln)[0] = _shaderDebug.PplnFindMaxValue1.Get();
    (*pPpln)[1] = _shaderDebug.PplnFindMaxValue2.Get();
    (*pPpln)[2] = _shaderDebug.PplnTimemapRemap.Get();
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
u32 FVulkanResourceManager::DebugShaderStorageSize(EShaderStages stages, EShaderDebugMode mode) {
    switch (mode) {
    case EShaderDebugMode::Trace:
    case EShaderDebugMode::Profiling:
        if (stages ^ EShaderStages::AllGraphics)
            return sizeof(u32) * 4; // fragCoord, (padding), position
        if (stages ^ EShaderStages::Compute)
            return sizeof(u32) * 4; // global invocation, position
        if (stages ^ EShaderStages::AllRayTracing)
            return sizeof(u32) * 4; // launch, position
        break;

    case EShaderDebugMode::Timemap:
        return sizeof(u32) * 4; // tile size, width, (padding)

    default: break;
    }
    AssertNotImplemented();
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanResourceManager::TearDownShaderDebuggerResources_() {
    if (_shaderDebug.PplnFindMaxValue1)
        ReleaseResource(_shaderDebug.PplnFindMaxValue1.Release());
    if (_shaderDebug.PplnFindMaxValue2)
        ReleaseResource(_shaderDebug.PplnFindMaxValue2.Release());
    if (_shaderDebug.PplnTimemapRemap)
        ReleaseResource(_shaderDebug.PplnTimemapRemap.Release());

    _shaderDebug.dsLayoutCaches.clear_ReleaseMemory();
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanResourceManager::CreateFindMaxValuePipeline1_() {
    if (_shaderDebug.PplnFindMaxValue1)
        return true;

    FString source{MakeStringView(R"#(
layout (local_size_x_id = 0, local_size_y = 1, local_size_z = 1) in;

layout (binding = 0, std430) readonly buffer un_Timemap {
    uvec2 maxValue;
    uvec2 dimension;
    uvec2 pixels[];
};
layout (binding = 1, std430) buffer un_MaxValues {
    uvec2 line[];
};

bool Greater(uvec2 lhs, uvec2 rhs) {
    return (lhs.x == rhs.x ? lhs.y > rhs.y : lhs.x > rhs.x);
}

void FindMaxValue1() {
    uvec2 mv = uvec2(0);

    for (uint x = 0; x < dimension.x; ++x) {
        uint y = x + dimension.x * gl_GlobalInvocationID.x;
        uvec2 v = pixels[y];
        if (Greater(v, mv))
            mv = v;
    }

    line[gl_GlobalInvocationID.x] = mv;
}
)#")};

    FComputePipelineDesc desc;
    ONLY_IF_RHIDEBUG(const FConstChar debugName{ "dbg_FindMaxValuePipeline1" });
    desc.AddShader(EShaderLangFormat::Vulkan_110, "FindMaxValue1", std::move(source) ARGS_IF_RHIDEBUG(debugName));

    _shaderDebug.PplnFindMaxValue1 = FCPipelineID{ CreatePipeline(desc ARGS_IF_RHIDEBUG(debugName)) };
    return (!!_shaderDebug.PplnFindMaxValue1);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanResourceManager::CreateFindMaxValuePipeline2_() {
    if (_shaderDebug.PplnFindMaxValue2)
        return true;

    FString source{MakeStringView(R"#(
layout (local_size_x_id = 0, local_size_y = 1, local_size_z = 1) in;

layout (binding = 0, std430) buffer un_Timemap {
    coherent uvec2 maxValue;
    readonly uvec2 dimension;
    readonly uvec2 pixels[];
};
layout (binding = 1, std430) readonly buffer un_MaxValues {
    uvec2 line[];
};

bool Greater(uvec2 lhs, uvec2 rhs) {
    return (lhs.x == rhs.x ? lhs.y > rhs.y : lhs.x > rhs.x);
}

void FindMaxValue2() {
    uvec2 mv = uvec2(0);

    for (uint y = 0; y < dimension.y; ++y) {
        uvec2 v = line[y];
        if (Greater(v, mv))
            mv = v;
    }

    maxValue = mv;
}
)#")};

    FComputePipelineDesc desc;
    ONLY_IF_RHIDEBUG(const FConstChar debugName{ "dbg_FindMaxValuePipeline2" });
    desc.AddShader(EShaderLangFormat::Vulkan_110, "FindMaxValue2", std::move(source) ARGS_IF_RHIDEBUG(debugName));

    _shaderDebug.PplnFindMaxValue2 = FCPipelineID{ CreatePipeline(desc ARGS_IF_RHIDEBUG(debugName)) };
    return (!!_shaderDebug.PplnFindMaxValue2);
}
#endif
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
bool FVulkanResourceManager::CreateTimemapRemapPipeline_() {
    if (_shaderDebug.PplnTimemapRemap)
        return true;

    FString source{MakeStringView(R"#(
layout (local_size_x_id = 0, local_size_y_id = 1, local_size_z = 1) in;

layout(binding = 0, std430) buffer un_Timemap {
    readonly uvec2 maxValue;
    readonly ivec2 dimension;
    readonly uvec2 pixels[];
};

layout(binding = 1) writeonly uniform image2D un_OutImage;

double ToDouble (uvec2 v) {
    return double(v.x) + double(v.y) * double(0xFFFFFFFF);
}

vec3 HeatMapColor( float x ) {
    x = clamp( x, 0.0, 1.0 );
    vec4 x1 = vec4( 1.0, x, x * x, x * x * x ); // 1 x x2 x3
    vec4 x2 = x1 * x1.w * x; // x4 x5 x6 x7
    return vec3(
        dot( x1.xyzw, vec4( -0.027780558, +1.228188385, +0.278906882, +3.892783760 ) ) + dot( x2.xy, vec2( -8.490712758, +4.069046086 ) ),
        dot( x1.xyzw, vec4( +0.014065206, +0.015360518, +1.605395918, -4.821108251 ) ) + dot( x2.xy, vec2( +8.389314011, -4.193858954 ) ),
        dot( x1.xyzw, vec4( -0.019628385, +3.122510347, -5.893222355, +2.798380308 ) ) + dot( x2.xy, vec2( -3.608884658, +4.324996022 ) ) );
}

void Remap() {
    double mv = ToDouble( maxValue );
    double vsum = 0.0;
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = ivec2(gl_WorkGroupSize.xy * gl_NumWorkGroups.xy);
    vec2 ncoord	= vec2(coord) / vec2(size);
    ivec2 px_coord = ivec2(ncoord * vec2(dimension));
    ivec2 px_size = ivec2(vec2(size) / vec2(dimension) + 0.5);

    for (int y = 0; y < px_size.y; ++y)
    for (int x = 0; x < px_size.x; ++x)  {
        double	v = ToDouble( pixels[ (px_coord.x + x) + (px_coord.y + y) * dimension.x ] );
        vsum += v;
    }

    vsum /= double(px_size.x * px_size.y);

    imageStore( un_OutImage, coord, vec4(HeatMapColor( float(vsum / mv) ), 1.0 ));
}
)#")};

    FComputePipelineDesc desc;
    ONLY_IF_RHIDEBUG(const FConstChar debugName{ "dbg_TimemapRemapPipeline" });
    desc.AddShader(EShaderLangFormat::Vulkan_110, "Remap", std::move(source) ARGS_IF_RHIDEBUG(debugName));

    _shaderDebug.PplnTimemapRemap = FCPipelineID{ CreatePipeline(desc ARGS_IF_RHIDEBUG(debugName)) };
    return (!!_shaderDebug.PplnTimemapRemap);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
