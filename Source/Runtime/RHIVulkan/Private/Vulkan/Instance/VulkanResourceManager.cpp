#include "stdafx.h"

#include "Vulkan/Instance/VulkanResourceManager.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Pipeline/VulkanShaderModule.h"

#include "RHI/EnumToString.h"
#include "RHI/PipelineCompiler.h"

#include "Diagnostic/Logger.h"
#include "External/vulkan/Vulkan-Header.git/include/vulkan/vulkan_android.h"
#include "IO/FormatHelpers.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
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
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanResourceManager::FVulkanResourceManager(const FVulkanDevice& device)
    : _device(device)
      , _memoryManager(device)
      , _descriptorManager(device)
      , _submissionCounter(0) { }

//----------------------------------------------------------------------------
FVulkanResourceManager::~FVulkanResourceManager() { }
//----------------------------------------------------------------------------
bool FVulkanResourceManager::Construct() {
    if (not _memoryManager.Construct()) {
        LOG(RHI, Error, L"failed to initialize vulkan memory manager");
        return false;
    }
    if (not _descriptorManager.Construct()) {
        LOG(RHI, Error, L"failed to initialize vulkan descriptor manager");
        return false;
    }
    if (not CreateEmptyDescriptorSetLayout_(&_emptyDSLayout)) {
        LOG(RHI, Error, L"failed to create empty descriptor set layout");
        return false;
    }
    if (not CheckHostVisibleMemory_()) {
        LOG(RHI, Error, L"failed to check host visible memory");
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
void FVulkanResourceManager::TearDown() {
    TearDownStagingBuffers_();
    ONLY_IF_RHIDEBUG(TearDownShaderDebuggerResources_());

    // release all pools (should all be empty !)

    _resources.ImagePool.Clear_AssertCompletelyEmpty();
    _resources.BufferPool.Clear_AssertCompletelyEmpty();
    _resources.MemoryObjectPool.Clear_AssertCompletelyEmpty();
    _resources.GPipelinePool.Clear_AssertCompletelyEmpty();
    _resources.CPipelinePool.Clear_AssertCompletelyEmpty();
    _resources.MPipelinePool.Clear_AssertCompletelyEmpty();
    _resources.RPipelinePool.Clear_AssertCompletelyEmpty();
    _resources.RtGeometryPool.Clear_AssertCompletelyEmpty();
    _resources.RtScenePool.Clear_AssertCompletelyEmpty();
    _resources.RtShaderTablePool.Clear_AssertCompletelyEmpty();
    _resources.SwapchainPool.Clear_AssertCompletelyEmpty();

    // release all cached resources

    TearDownCache_(_resources.SamplerCache);
    TearDownCache_(_resources.PplnLayoutCache);
    TearDownCache_(_resources.DslayoutCache);
    TearDownCache_(_resources.RenderPassCache);
    TearDownCache_(_resources.FramebufferCache);
    TearDownCache_(_resources.PplnResourcesCache);

    // release shader cache
    {
        const FCriticalScope shaderCacheLock{&_shaderCacheCS};

        for (PVulkanShaderModule& sh : _shaderCache) {
            Assert(sh);
            Assert_NoAssume(sh->RefCount() == 1);

            checked_cast<FVulkanShaderModule>(sh)->TearDown(_device);
        }

        _shaderCache.clear_ReleaseMemory();
    }

    // release pipeline compilers
    {
        const FReadWriteLock::FScopeLockWrite compilerLockWrite{_compilersRW};

        _compilers.clear_ReleaseMemory();
    }

    _descriptorManager.TearDown();
    _memoryManager.TearDown();
}

//----------------------------------------------------------------------------
void FVulkanResourceManager::AddCompiler(const PPipelineCompiler& pCompiler) {
    Assert(pCompiler);

    const FReadWriteLock::FScopeLockWrite compilerLockWrite{_compilersRW};

    _compilers.insert(pCompiler);
}

//----------------------------------------------------------------------------
void FVulkanResourceManager::OnSubmit() {
    _submissionCounter.fetch_add(1, std::memory_order_relaxed);
}

//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks>
void FVulkanResourceManager::TearDownCache_(TCache<T, _ChunkSize, _MaxChunks>& cache) {
    cache.Clear_IgnoreLeaks([this](T* pCached) {
        pCached->TearDown(*this);
        return true;
    });
}
//----------------------------------------------------------------------------
// CreateDescriptorSetLayout
//----------------------------------------------------------------------------
FRawDescriptorSetLayoutID FVulkanResourceManager::CreateDescriptorSetLayout(const FPipelineDesc::FSharedUniformMap& uniforms ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    FRawDescriptorSetLayoutID result;
    TResourceProxy<FVulkanDescriptorSetLayout>* setLayout{ nullptr };
    VerifyRelease( CreateDescriptorSetLayout_(&result, &setLayout, uniforms ARGS_IF_RHIDEBUG(debugName)) );
    Assert_NoAssume(result.Valid());
    return result;
}
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CreateEmptyDescriptorSetLayout_(FRawDescriptorSetLayoutID* pId) {
    Assert(pId);

    FVulkanDescriptorSetLayout::FBindings bindings;
    FPipelineDesc::FSharedUniformMap uniforms = NEW_REF(RHIResource, FPipelineDesc::FUniformMap);

    TResourceProxy<FVulkanDescriptorSetLayout> emptyKey{ &bindings, uniforms };
    if (Likely(CreateCachedResource_(pId, std::move(emptyKey), _device, std::move(bindings) ARGS_IF_RHIDEBUG("EmptyDSLayout")).first))
        return true;

    LOG(RHI, Error, L"failed to create an empty descriptor set layout");
    return false;
}
//----------------------------------------------------------------------------
// CreatePipelineLayout
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CreatePipelineLayout_(
    FRawPipelineLayoutID* pId,
    const TResourceProxy<FVulkanPipelineLayout>** pPplnLayoutRef,
    FPipelineDesc::FPipelineLayout&& desc
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {

    // init pipeline layout mode
    FDSLayouts dsLayouts;
    for (const FPipelineDesc::FDescriptorSet& ds : desc.DescriptorSets) {
        FRawDescriptorSetLayoutID dsId;
        TResourceProxy<FVulkanDescriptorSetLayout>* dsLayout{ nullptr };
        VerifyRelease( CreateDescriptorSetLayout_(&dsId, &dsLayout, ds.Uniforms ARGS_IF_RHIDEBUG(debugName)) );

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
    const auto it = pool.FindOrAdd(std::move(layout), [this ARGS_IF_RHIDEBUG(debugName)](TResourceProxy<FVulkanPipelineLayout>* pLayout, FIndex , bool exist) -> bool {
        if (not exist && not pLayout->Construct(_device, ResourceData(_emptyDSLayout).Read()->Layout ARGS_IF_RHIDEBUG(debugName)))
            return false;
        pLayout->AddRef();
        return true;
    });

    *pPplnLayoutRef = pool[it.first];

    if (Likely(!!*pPplnLayoutRef)) {
        *pId = FRawPipelineLayoutID{ it.first, (*pPplnLayoutRef)->InstanceID() };

        if (it.second) {
            // release allocated resources if the ds was already cached
            for (auto& ds : dslayouts)
                ReleaseResource(ds.first);
        }

        Assert_NoAssume(pId->Valid());
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
    const FPipelineDesc::FSharedUniformMap& uniforms
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(pId);
    Assert(pDSLayout);
    Assert(uniforms);

    FVulkanDescriptorSetLayout::FBindings bindings;
    TResourceProxy<FVulkanDescriptorSetLayout> emptyKey{ &bindings, uniforms };

    *pDSLayout = CreateCachedResource_(pId, std::move(emptyKey), _device, std::move(bindings) ARGS_IF_RHIDEBUG(debugName)).first;
    if (Likely(!!*pDSLayout))
        return true;

    ONLY_IF_ASSERT(LOG(RHI, Error, L"failed to create descriptor set layout for '{0}'", debugName));
    return false;
}
//----------------------------------------------------------------------------
// CompilerShaders
//----------------------------------------------------------------------------
template <typename _Desc>
bool FVulkanResourceManager::CompileShaders_(_Desc& desc) {
    const EShaderLangFormat required = (_device.vkVersion() | EShaderLangFormat::ShaderModule);

    // try to use external compilers
    {
        const FReadWriteLock::FScopeLockRead compilersLock{ _compilersRW };

        for (const PPipelineCompiler& compiler : _compilers) {
            if (compiler->IsSupported(desc, required))
                return compiler->Compile(desc, required);
        }
    }

    // check if shaders supported by default compiler
    const auto formats = BuiltinFormats_(_device);

    bool isSupported = true;
    for (auto& sh : desc.Shaders) {
        if (sh.second.Sources.empty())
            continue;

        bool found = false;
        for (EShaderLangFormat fmt : formats) {
            const auto it = sh.second.Sources.find(fmt);
            if (sh.second.Sources.end() == it)
                continue;

            if (fmt & EShaderLangFormat::ShaderModule) {
                PShaderSource source = it->second;
                sh.second.Sources.clear();
                sh.second.Sources.insert({ fmt, std::move(source) });

                found = true;
                break;
            }

            if (fmt & EShaderLangFormat::SPIRV) {
                PVulkanShaderModule module;
                if (not CompileShaderSPIRV_(&module, it->second))
                    return false;

                Assert(module);
                sh.second.Sources.clear();
                sh.second.AddSource(fmt, module);

                found = true;
                break;
            }
        }

        CLOG(not found, RHI, Warning, L"unsupported shader format: {0}", sh.first);
        isSupported &= found;
    }

    return isSupported;
}
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CompileShaders_(FComputePipelineDesc& desc) {
    const EShaderLangFormat required = (_device.vkVersion() | EShaderLangFormat::ShaderModule);

    // try to use external compilers
    {
        const FReadWriteLock::FScopeLockRead compilersLock{ _compilersRW };

        for (const PPipelineCompiler& compiler : _compilers) {
            if (compiler->IsSupported(desc, required))
                return compiler->Compile(desc, required);
        }
    }

    // check if shaders supported by default compiler
    const auto formats = BuiltinFormats_(_device);

    for (EShaderLangFormat fmt : formats) {
        const auto it = desc.Shader.Sources.find(fmt);
        if (desc.Shader.Sources.end() == it)
            continue;

        if (fmt & EShaderLangFormat::ShaderModule) {
            PShaderData<FShaderSource> source = it->second;
            desc.Shader.Sources.clear();
            desc.Shader.Sources.insert({ fmt, std::move(source) });
            return true;
        }

        if (fmt & EShaderLangFormat::SPIRV) {
            PVulkanShaderModule module;
            if (not CompileShaderSPIRV_(&module, it->second))
                return false;

            Assert(module);
            desc.Shader.Sources.clear();
            desc.Shader.AddSource(fmt, module);
            return true;
        }
    }

    LOG(RHI, Error, L"unsupported shader formats: {0}", Fmt::Join(desc.Shader.Sources.Keys(), L", "));
    return false;
}
//----------------------------------------------------------------------------
bool FVulkanResourceManager::CompileShaderSPIRV_(PVulkanShaderModule* pVkShaderModule, const PShaderSource& source) {
    const FRawData* const pRawData =  std::get_if<FRawData>(source->Data());
    if (not pRawData || pRawData->empty()) {
        LOG(RHI, Error, L"invalid shader data format");
        return false;
    }

    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = pRawData->SizeInBytes();
    info.pCode = reinterpret_cast<const uint32_t*>(pRawData->data());

    VkShaderModule vkShaderModule = VK_NULL_HANDLE;
    VK_CHECK( _device.vkCreateShaderModule(
        _device.vkDevice(),
        &info,
        _device.vkAllocator(),
        &vkShaderModule) );
    Assert_NoAssume(VK_NULL_HANDLE != vkShaderModule);

#if USE_PPE_RHITASKNAME
    _device.SetObjectName(
        reinterpret_cast<u64>(vkShaderModule),
        source->DebugName(),
        VK_OBJECT_TYPE_SHADER_MODULE );
#endif

    *pVkShaderModule = NEW_REF(RHIShader, FVulkanShaderModule,
        vkShaderModule,
        source->HashValue(),
        source->EntryPoint()
        ARGS_IF_RHIDEBUG(source->DebugName()) );

    const FCriticalScope shaderCacheLock{ &_shaderCacheCS };
    _shaderCache.push_back(*pVkShaderModule);

    return true;
}
//----------------------------------------------------------------------------
// CreatePipeline
//----------------------------------------------------------------------------
template <u32 _Uid, typename _Desc>
bool FVulkanResourceManager::CreateDevicePipeline_(details::TResourceId<_Uid>* pId, _Desc& desc ARGS_IF_RHIDEBUG(FStringView pipelineType, FConstChar debugName)) {
    if (not CompileShaders_(desc)) {
        RHI_LOG(Error, L"failed to compile shaders for {1} pipeline '{0}'", debugName, pipelineType);
        return false;
    }

    FRawPipelineLayoutID layoutId;
    const TResourceProxy<FVulkanPipelineLayout>* pLayout{ nullptr };
    VerifyRelease( CreatePipelineLayout_(&layoutId, &pLayout, std::move(desc.PipelineLayout) ARGS_IF_RHIDEBUG(debugName)) );

    TPooledResource_<_Uid>* const pPipeline = CreatePooledResource_(pId);
    Assert(pPipeline);

    if (Unlikely(not pPipeline->Construct(desc, layoutId ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct {1} pipeline '{0}'", debugName, pipelineType);
        Verify( ReleaseResource_(ResourcePool_(*pId), pPipeline, pId->Index, 0) );
        *pId = Default;
        return false;
    }

    Assert_NoAssume(pId->Valid());
    pLayout->AddRef();
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
        RHI_LOG(Error, L"failed to create memory object for image '{0}'", debugName);
        return Default;
    }
    Assert_NoAssume(memoryId.Valid());

    FRawImageID imageId;
    TResourceProxy<FVulkanImage>* const pImage = CreatePooledResource_(&imageId);
    Assert(pImage);

    if (Unlikely(not pImage->Construct(*this, desc, memoryId, pMemory->Data(), queues, defaultState ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct image '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(imageId), pImage, imageId.Index, 0) );
        Verify( ReleaseResource_(ResourcePool_(memoryId), pMemory, memoryId.Index, 0) );
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
    TMemoryView<const u32> queueFamilyIndices
    ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    Assert(externalImage);
    Assert(onRelease);

    FRawImageID imageId;
    TResourceProxy<FVulkanImage>* const pImage = CreatePooledResource_(&imageId);
    Assert(pImage);

    if (Unlikely(not pImage->Construct(_device, desc, externalImage, std::move(onRelease), queueFamilyIndices ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct image from external resource '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(imageId), pImage, imageId.Index, 0) );
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
        RHI_LOG(Error, L"failed to create memory object for buffer '{0}'", debugName);
        return Default;
    }
    Assert_NoAssume(memoryId.Valid());

    FRawBufferID bufferId;
    TResourceProxy<FVulkanBuffer>* const pBuffer = CreatePooledResource_(&bufferId);
    Assert(pBuffer);

    if (Unlikely(not pBuffer->Construct(*this, desc, memoryId, pMemory->Data(), queues ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct buffer '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(bufferId), pBuffer, bufferId.Index, 0) );
        Verify( ReleaseResource_(ResourcePool_(memoryId), pMemory, memoryId.Index, 0) );
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
    TMemoryView<const u32> queueFamilyIndices,
    FConstChar debugName) {
    Assert(externalBuffer);
    Assert(onRelease);

    FRawBufferID bufferId;
    TResourceProxy<FVulkanBuffer>* const pBuffer = CreatePooledResource_(&bufferId);
    Assert(pBuffer);

    if (Unlikely(not pBuffer->Construct(_device, desc, externalBuffer, std::move(onRelease), queueFamilyIndices ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct buffer '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(bufferId), pBuffer, bufferId.Index, 0) );
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
    if (Likely(CreateCachedResource_(&samplerId, std::move(emptyKey), _device ARGS_IF_RHIDEBUG(debugName)).first)) {
        Assert_NoAssume(samplerId.Valid());
        return samplerId;
    }

    RHI_LOG(Error, L"failed to create sampler for '{0}", debugName);
    return Default;
}
//----------------------------------------------------------------------------
// CreateRenderPass
//----------------------------------------------------------------------------
FRawRenderPassID FVulkanResourceManager::CreateRenderPass(const TMemoryView<const FVulkanLogicalRenderPass*>& passes ARGS_IF_RHIDEBUG(FConstChar debugName)) {
    TResourceProxy<FVulkanRenderPass> emptyKey{ passes };

    FRawRenderPassID renderPassId;
    if (Likely(CreateCachedResource_(&renderPassId, std::move(emptyKey), _device ARGS_IF_RHIDEBUG(debugName)).first)) {
        Assert_NoAssume(renderPassId.Valid());
        return renderPassId;
    }

    RHI_LOG(Error, L"failed to create render pass for '{0}", debugName);
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

    Verify( AcquireResource(renderPass) );

    TResourceProxy<FVulkanFramebuffer> emptyKey{ attachments, renderPass, dim, layers };

    FRawFramebufferID framebufferId;
    const auto it = CreateCachedResource_(&framebufferId, std::move(emptyKey), *this ARGS_IF_RHIDEBUG(debugName));
    if (Likely(it.first)) {
        Assert_NoAssume(framebufferId.Valid());
        _validation.CreatedFramebuffers.fetch_add(it.second ? 0 : 1, std::memory_order_relaxed);
        return framebufferId;
    }

    RHI_LOG(Error, L"failed to create frame buffer for '{0}", debugName);
    ReleaseResource(renderPass);
    return Default;
}
//----------------------------------------------------------------------------
// CreateDescriptorSet
//----------------------------------------------------------------------------
const FVulkanPipelineResources* FVulkanResourceManager::CreateDescriptorSet(
    const FPipelineResources& desc, FVulkanCommandBatch::FResourceMap& resources ) {
    FRawPipelineResourcesID resourcesId = FPipelineResources::Cached(desc);

    if (Likely(resourcesId)) { // use cached resources ?
        TResourceProxy<FVulkanPipelineResources>* const pResources = ResourcePool_(resourcesId)[resourcesId.Index];
        Assert(pResources);

        if (pResources->InstanceID() == resourcesId.InstanceID) {
            if (resources.insert({ resourcesId.Pack(), 1 }).second)
                pResources->AddRef();

            Assert_NoAssume(pResources->Data().AllResourcesAlive(*this));
            return std::addressof(pResources->Data());
        }
    }

    TResourceProxy<FVulkanDescriptorSetLayout>* const pDSLayout = ResourcePool_(desc.Layout())[desc.Layout().Index];
    Assert_NoAssume( pDSLayout->IsCreated() && pDSLayout->InstanceID() == desc.Layout().InstanceID );

    TResourceProxy<FVulkanPipelineResources> emptyKey{ desc };
    const auto [pResources, exist] =
        CreateCachedResource_(&resourcesId, std::move(emptyKey), *this);

    if (Likely(pResources)) {
        Assert_NoAssume(resourcesId.Valid());
        if (Unlikely(not exist)) {
            pDSLayout->AddRef();
            _validation.CreatedPplnResources.fetch_add(1, std::memory_order_relaxed);
        }

        FPipelineResources::SetCached(desc, resourcesId);
        if (resources.insert({ resourcesId.Pack(), 1 }).second)
            pResources->AddRef();

        Assert_NoAssume(pResources->Data().AllResourcesAlive(*this));
        return std::addressof(pResources->Data());
    }

    LOG(RHI, Error, L"failed to create descriptor set layout");
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

    AssertReleaseMessage(L"dynamic offsets are not supported here", desc.DynamicOffsets().empty());

    TResourceProxy<FVulkanDescriptorSetLayout>* const pDSLayout = ResourcePool_(desc.Layout())[desc.Layout().Index];
    Assert_NoAssume( pDSLayout->IsCreated() && pDSLayout->InstanceID() == desc.Layout().InstanceID );

    TResourceProxy<FVulkanPipelineResources> emptyKey{ desc };
    const auto [pResources, exist] =
        CreateCachedResource_(&resourcesId, std::move(emptyKey), *this);

    if (Likely(pResources)) {
        Assert_NoAssume(resourcesId.Valid());
        if (Unlikely(not exist))
            pDSLayout->AddRef();
        FPipelineResources::SetCached(desc, resourcesId);
        return true;
    }

    LOG(RHI, Error, L"failed to cache descriptor set layout");
    return Default;
}
//----------------------------------------------------------------------------
// ReleaseResource
//----------------------------------------------------------------------------
void FVulkanResourceManager::ReleaseResource(FPipelineResources& desc) {
    if (const FRawPipelineResourcesID resourcesId = FPipelineResources::Cached(desc)) {
        FPipelineResources::SetCached(desc,FRawPipelineResourcesID{});
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
    Assert_NoAssume(_device.Enabled().RayTracingKHR);

    FRawMemoryID memoryId;
    TResourceProxy<FVulkanMemoryObject>* pMemory{ nullptr };
    if (Unlikely(not CreateMemory_(&memoryId, &pMemory, mem ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to create memory object for raytracing geometry for '{0}'", debugName);
        return Default;
    }
    Assert_NoAssume(memoryId.Valid());

    FRawRTGeometryID geometryId;
    TResourceProxy<FVulkanRTGeometry>* const pGeometry = CreatePooledResource_(&geometryId);

    if (Unlikely(not pGeometry->Construct(*this, desc, memoryId, pMemory->Data() ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct raytracing geometry '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(geometryId), pGeometry, geometryId.Index, 0) );
        Verify( ReleaseResource_(ResourcePool_(memoryId), pMemory, memoryId.Index, 0) );
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
    Assert_NoAssume(_device.Enabled().RayTracingKHR);

    FRawMemoryID memoryId;
    TResourceProxy<FVulkanMemoryObject>* pMemory{ nullptr };
    if (Unlikely(not CreateMemory_(&memoryId, &pMemory, mem ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to create memory object for raytracing scene for '{0}'", debugName);
        return Default;
    }
    Assert_NoAssume(memoryId.Valid());

    FRawRTSceneID sceneId;
    TResourceProxy<FVulkanRTScene>* const pScene = CreatePooledResource_(&sceneId);

    if (Unlikely(not pScene->Construct(*this, desc, memoryId, pMemory->Data() ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct raytracing scene '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(sceneId), pScene, sceneId.Index, 0) );
        Verify( ReleaseResource_(ResourcePool_(memoryId), pMemory, memoryId.Index, 0) );
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
FRawRTShaderTableID FVulkanResourceManager::CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(FStringView debugName)) {
    FRawRTShaderTableID shaderTableId;
    TResourceProxy<FVulkanRayTracingShaderTable>* const pShaderTable = CreatePooledResource_(&shaderTableId);
    Assert(pShaderTable);

    if (Unlikely(not pShaderTable->Construct(ARG0_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct raytracing shader table '{0}'", debugName);
        Verify( ReleaseResource_(ResourcePool_(shaderTableId), pShaderTable, shaderTableId.Index, 0) );
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
    const FVulkanSwapchain* swapchain = ResourceDataIFP(oldSwapchain, false, true);
    if (Likely(swapchain)) {
        Assert_NoAssume(oldSwapchain.Valid());

        if (Unlikely(not const_cast<FVulkanSwapchain*>(swapchain)->ReConstruct(fg, desc ARGS_IF_RHIDEBUG(debugName)))) {
            RHI_LOG(Error, L"failed to re-construct old swapchain for '{0}'", debugName);
            return Default;
        }

        return oldSwapchain;
    }

    FRawSwapchainID swapchainId;
    TResourceProxy<FVulkanSwapchain>* const pSwapchain = CreatePooledResource_(&swapchainId);
    Assert_NoAssume(pSwapchain);

    if (Unlikely(not pSwapchain->Construct(fg, desc ARGS_IF_RHIDEBUG(debugName)))) {
        RHI_LOG(Error, L"failed to construct swapchain for '{0}'", debugName);
        VerifyRelease( ReleaseResource_(ResourcePool_(swapchainId), pSwapchain, swapchainId.Index, 0) );
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
    STATIC_CONST_INTEGRAL(u32, Scale, MaxCached / 16);

    const auto updateCounter = [](std::atomic<u32>* pCounter, u32 maxValue) NOEXCEPT -> u32 {
        if (0 == maxValue)
            return 0;

        u32 count = 0;
        for (u32 expected = 0; not pCounter->compare_exchange_weak(expected, expected - count, std::memory_order_relaxed);)
            count = Min(maxValue, expected * Scale);

        return count;
    };

    const auto updateLastIndex = [](std::atomic<u32>* pLastIndex, u32 count, u32 size) NOEXCEPT -> u32 {
        u32 expected = 0;
        for (u32 newValue = count; not pLastIndex->compare_exchange_weak(expected, newValue, std::memory_order_relaxed);) {
            newValue = expected + count;
            newValue = (newValue >= size ? newValue - size : newValue);
            Assert(newValue < size);
        }

        return expected;
    };

    const auto validateResources = [&, this, maxIteration](std::atomic<u32>* pCounter, std::atomic<u32>* pLastIndex, auto& pool) NOEXCEPT -> void {
        if (const u32 newMaxIteration = updateCounter(pCounter, maxIteration)) {
            const u32 maxCount = checked_cast<u32>(pool.NumLiveBlocks());
            const u32 lastIndex = updateLastIndex(pLastIndex, newMaxIteration, maxCount);

            forrange(i, 0, newMaxIteration) {
                u32 j = lastIndex + i;
                j  = (j >= maxCount ? j - maxCount : j);

                const FIndex index{ checked_cast<FIndex>(j) };
                auto* const pResource = pool[index];
                Assert(pResource);

                if (pResource->IsCreated() && not pResource->Data().AllResourcesAlive(*this))
                    ReleaseResource_(pool, pResource, index, pResource->RefCount());
            }
        }
    };

    validateResources(&_validation.CreatedPplnResources, &_validation.LastCheckedPplnResource, _resources.PplnResourcesCache);
    validateResources(&_validation.CreatedFramebuffers, &_validation.LastCheckedFramebuffer, _resources.FramebufferCache);
}
//----------------------------------------------------------------------------
// ReleaseMemory
//----------------------------------------------------------------------------
void FVulkanResourceManager::ReleaseMemory() {
    // will reclaim memory from the various caches (pools are always shrink to fit)
    TearDownStagingBuffers_();

    TearDownCache_(_resources.SamplerCache);
    TearDownCache_(_resources.PplnLayoutCache);
    TearDownCache_(_resources.DslayoutCache);
    TearDownCache_(_resources.RenderPassCache);
    TearDownCache_(_resources.FramebufferCache);
    TearDownCache_(_resources.PplnResourcesCache);
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
    ONLY_IF_ASSERT(FConstChar debugName);

    FBufferDesc desc;
    desc.Usage = usage;

    switch (usage) {
    case EBufferUsage::TransferSrc:
        pool = &_staging.Write;
        desc.SetSize(_staging.WritePageSize);
        memType = EMemoryType::HostWrite;
        poolIndexMask = 1u << 30;
        ONLY_IF_ASSERT(debugName = "HostWriteBuffer");
        break;

    case EBufferUsage::TransferDst:
        pool = &_staging.Read;
        desc.SetSize(_staging.ReadPageSize);
        memType = EMemoryType::HostRead;
        poolIndexMask = 2u << 30;
        ONLY_IF_ASSERT(debugName = "HostReadBuffer");
        break;

    case EBufferUsage::Uniform:
        pool = &_staging.Uniform;
        desc.SetSize(_staging.UniformPageSize);
        memType = EMemoryType::HostWrite;
        poolIndexMask = 3u << 30;
        ONLY_IF_ASSERT(debugName = "HostUniformBuffer");
        break;

    default: AssertReleaseFailed(L"unsupported buffer usage");
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

    ONLY_IF_ASSERT(LOG(RHI, Error, L"failed to allocate staging buffer for '{0}'", debugName));
    return false;
}
//----------------------------------------------------------------------------
// ResourceDescription
//----------------------------------------------------------------------------

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
    AssertMessage(L"resource pool overflow", pResource);

    Meta::Construct(pResource, std::forward<_Args>(args)...);
    *pId = details::TResourceId<_Uid>{ index, pResource->InstanceID() };

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
    const TPair<FIndex, bool> it = pool.FindOrAdd(std::move(rkey), [&](TPooledResource_<_Uid>* pResource, FIndex , bool exist) {
        if (Unlikely(not exist && not pResource->Construct(std::forward<_Args>(args)...)))
            return false;
        pResource->AddRef();
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
        RHI_LOG(Error, L"failed to construct memory object '{0}'", debugName);
        VerifyRelease( ReleaseResource_(ResourcePool_(*pId), *pMemory, pId->Index, 0) );
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
            uniformHeapSize += props.memoryHeaps[i].size;
        if (cachedHeaps.test(i) || coherentHeaps.test(i))
            transferHeapSize += props.memoryHeaps[i].size;
    }

    size_t uniformSize = (uniformHeapSize / FStagingBufferPool::Capacity);
    size_t transferSize = (transferHeapSize / FStagingBufferPool::Capacity);

#if defined(PLATFORM_PC) || defined(PLATFORM_MOBILE)
    // keep some slack for other processes
    if (uniformSize > 128_MiB)  uniformSize = 32_MiB; else
    if (uniformSize >  64_MiB)  uniformSize = 16_MiB; else
                                uniformSize =  8_MiB;

    if (transferSize > 512_MiB) transferSize = 256_MiB; else
    if (transferSize > 128_MiB) transferSize = 128_MiB; else
                                transferSize =  64_MiB;
#endif

    _staging.WritePageSize = _staging.ReadPageSize = checked_cast<u32>(transferSize);
    _staging.UniformPageSize = checked_cast<u32>(uniformSize);
    return true;
}
//----------------------------------------------------------------------------
// Debugger
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FRawDescriptorSetLayoutID FVulkanResourceManager::CreateDebugDescriptorSetLayout(
    EShaderDebugMode debugMode,
    EShaderStages debuggableShaders,
    FConstChar debugName ) {
    const u32 key = ((static_cast<u32>(debuggableShaders) & 0xFFFFFFu) | (static_cast<u32>(debugMode) << 24));
    const auto it = _shaderDebug.dsLayoutCaches.find(key);
    if (_shaderDebug.dsLayoutCaches.end() != it)
        return it->second;

    FPipelineDesc::FStorageBuffer sbDesc;
    sbDesc.DynamicOffsetIndex = 0;
    sbDesc.ArrayStride = sizeof(u32);
    sbDesc.StaticSize = DebugShaderStorageSize(debuggableShaders, debugMode);
    sbDesc.State = EResourceState_FromShaders(debuggableShaders) |
        EResourceState::ShaderReadWrite |
        EResourceState::_BufferDynamicOffset;

    FPipelineDesc::FVariantUniform sbUniform;
    sbUniform.ArraySize = 1;
    sbUniform.Data = sbDesc;
    sbUniform.StageFlags = debuggableShaders;
    sbUniform.Index = FBindingIndex{ UMax, 0 };

    FPipelineDesc::FSharedUniformMap uniforms;
    uniforms->Add_AssertUnique(FUniformID{ "dbg_ShaderTrace" }, sbUniform);

    const FRawDescriptorSetLayoutID layout = CreateDescriptorSetLayout(uniforms, debugName);
    Assert_NoAssume(layout.Valid());

    _shaderDebug.dsLayoutCaches.insert_AssertUnique({ key, layout });
    return layout;
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

        auto& dsLayout = dsPool.Value(checked_cast<FIndex>(src.second.Index));
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
        const FRawDescriptorSetLayoutID dsLayoutId = CreateDebugDescriptorSetLayout(debugMode, debuggableShaders, origin.DebugName());
        Assert(dsLayoutId.Valid());

        auto& dsLayout = dsPool.Value(checked_cast<FIndex>(dsLayoutId.Index));
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
