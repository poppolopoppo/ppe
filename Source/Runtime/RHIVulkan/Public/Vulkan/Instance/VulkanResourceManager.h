#pragma once

#include "Vulkan/Vulkan_fwd.h"

#include "Vulkan/VulkanCommon.h"
#include "Vulkan/Buffer/VulkanBuffer.h"
#include "Vulkan/Descriptors/VulkanDescriptorManager.h"
#include "Vulkan/Descriptors/VulkanDescriptorSetLayout.h"
#include "Vulkan/Descriptors/VulkanPipelineResources.h"
#include "Vulkan/Instance/VulkanSwapchain.h"
#include "Vulkan/Image/VulkanImage.h"
#include "Vulkan/Image/VulkanSampler.h"
#include "Vulkan/Memory/VulkanMemoryManager.h"
#include "Vulkan/Memory/VulkanMemoryObject.h"
#include "Vulkan/Pipeline/VulkanComputePipeline.h"
#include "Vulkan/Pipeline/VulkanGraphicsPipeline.h"
#include "Vulkan/Pipeline/VulkanMeshPipeline.h"
#include "Vulkan/Pipeline/VulkanPipelineLayout.h"
#include "Vulkan/Pipeline/VulkanRayTracingPipeline.h"
#include "Vulkan/RayTracing/VulkanRayTracingGeometry.h"
#include "Vulkan/RayTracing/VulkanRayTracingScene.h"
#include "Vulkan/RayTracing/VulkanRayTracingShaderTable.h"
#include "Vulkan/RenderPass/VulkanFrameBuffer.h"
#include "Vulkan/RenderPass/VulkanRenderPass.h"

#include "RHI/ResourceId.h"

#include "Container/HashMap.h"
#include "Container/HashSet.h"
#include "Memory/CachedMemoryPool.h"
#include "Memory/MemoryPool.h"
#include "RHI/ResourceProxy.h"
#include "Thread/AtomicPool.h"
#include "Vulkan/Command/VulkanCommandBatch.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVulkanResourceManager : Meta::FNonCopyable {
public:
    using FIndex = FRawImageID::FIndex;

    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    using TPool = TTypedMemoryPool<T, _ChunkSize, _MaxChunks, ALLOCATOR(RHIResource)>;
    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    using TCache = TCachedMemoryPool<T, T, _ChunkSize, _MaxChunks, ALLOCATOR(RHIResource)>;

    STATIC_CONST_INTEGRAL(u32, MaxImages,       1u << 10);
    STATIC_CONST_INTEGRAL(u32, MaxBuffers,      1u << 10);
    STATIC_CONST_INTEGRAL(u32, MaxMemoryObjs,   1u << 10);
    STATIC_CONST_INTEGRAL(u32, MaxCached,       1u << 9 );
    STATIC_CONST_INTEGRAL(u32, MaxRTObjects,    1u << 9 );

    using FImagePool            = TPool<    TResourceProxy<FVulkanImage>,                   MaxImages,      32  >;
    using FBufferPool           = TPool<    TResourceProxy<FVulkanBuffer>,                  MaxBuffers,     32  >;
    using FMemoryObjectPool     = TPool<    TResourceProxy<FVulkanMemoryObject>,            MaxMemoryObjs,  63  >;

    using FSamplerCache         = TCache<   TResourceProxy<FVulkanSampler>,                 MaxCached,      8   >;

    using FGPipelinePool        = TPool<    TResourceProxy<FVulkanGraphicsPipeline>,        MaxCached,      8   >;
    using FCPipelinePool        = TPool<    TResourceProxy<FVulkanComputePipeline>,         MaxCached,      8   >;
    using FMPipelinePool        = TPool<    TResourceProxy<FVulkanMeshPipeline>,            MaxCached,      8   >;
    using FRPipelinePool        = TPool<    TResourceProxy<FVulkanRayTracingPipeline>,      MaxCached,      8   >;

    using FPplnLayoutCache      = TCache<   TResourceProxy<FVulkanPipelineLayout>,          MaxCached,      8   >;
    using FDSLayoutCache        = TCache<   TResourceProxy<FVulkanDescriptorSetLayout>,     MaxCached,      8   >;
    using FPplnResourcesCache   = TCache<   TResourceProxy<FVulkanPipelineResources>,       MaxCached,      8   >;

    using FRTGeometryPool       = TPool<    TResourceProxy<FVulkanRayTracingGeometry>,      MaxRTObjects,   16  >;
    using FRTScenePool          = TPool<    TResourceProxy<FVulkanRayTracingScene>,         MaxRTObjects,   16  >;
    using FRTShaderTablePool    = TPool<    TResourceProxy<FVulkanRayTracingShaderTable>,   MaxRTObjects,   16  >;

    using FRenderPassCache      = TCache<   TResourceProxy<FVulkanRenderPass>,              MaxCached,      8   >;
    using FFramebufferCache     = TCache<   TResourceProxy<FVulkanFramebuffer>,             MaxCached,      8   >;

    using FSwapchainPool        = TPool<    TResourceProxy<FVulkanSwapchain>,               64,             1   >;

    using FStagingBufferPool    = TAtomicPool<FBufferID, 2, u32>;

    using FPipelineCompilers    = HASHSET(RHIResource, PPipelineCompiler);
    using FVulkanShaderRefs     = VECTOR(RHIResource, PVulkanShaderModule);
    using FDSLayouts            = TFixedSizeStack<TPair<
        FRawDescriptorSetLayoutID,
        TResourceProxy<FVulkanDescriptorSetLayout>*
    >,  MaxDescriptorSets >;

#if USE_PPE_RHIDEBUG
    using FDebugLayoutCache = HASHMAP(RHIResource, u32, FRawDescriptorSetLayoutID);
#endif

    FVulkanResourceManager(
        const FVulkanDevice& device,
        size_t maxStagingBufferMemory,
        size_t stagingBufferSize );
    ~FVulkanResourceManager();

    const FVulkanDevice& Device() const { return _device; }
    FVulkanMemoryManager& MemoryManager() { return _memoryManager; }
    FVulkanDescriptorManager& DescriptorManager() { return _descriptorManager; }

    u32 SubmitIndex() const { return _submissionCounter.load(std::memory_order_relaxed); }

    size_t HostReadBufferSize() const { return _staging.ReadPageSize; }
    size_t HostWriteBufferSize() const { return _staging.WritePageSize; }
    size_t UniformBufferSize() const { return _staging.UniformPageSize; }

    NODISCARD bool Construct();
    void TearDown();

    void OnSubmit();

    NODISCARD FRawMPipelineID CreatePipeline(FMeshPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawGPipelineID CreatePipeline(FGraphicsPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawCPipelineID CreatePipeline(FComputePipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawRTPipelineID CreatePipeline(FRayTracingPipelineDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD FRawImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, EVulkanQueueFamilyMask queues, EResourceState defaultState ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawBufferID CreateBuffer(const FBufferDesc& desc, const FMemoryDesc& mem, EVulkanQueueFamilyMask queues ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD FRawImageID CreateImage(const FVulkanExternalImageDesc& desc, FOnReleaseExternalImage&& onRelease ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawBufferID CreateBuffer(const FVulkanExternalBufferDesc& desc, FOnReleaseExternalBuffer&& onRelease ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD FRawImageID CreateImage(
        const FImageDesc& desc,
        FExternalImage externalImage, FOnReleaseExternalImage&& onRelease,
        TMemoryView<const u32> queueFamilyIndices,
        EResourceState defaultState
        ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawBufferID CreateBuffer(
        const FBufferDesc& desc,
        FExternalBuffer externalBuffer, FOnReleaseExternalBuffer&& onRelease,
        TMemoryView<const u32> queueFamilyIndices
        ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD FRawSamplerID CreateSampler(const FSamplerDesc& desc  ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD FRawRenderPassID CreateRenderPass(const TMemoryView<const FVulkanLogicalRenderPass* const>& passes ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawFramebufferID CreateFramebuffer(const TMemoryView<const TPair<FRawImageID, FImageViewDesc>>& attachments, FRawRenderPassID renderPass, const uint2& dim, u32 layers ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD bool CacheDescriptorSet(FPipelineResources& desc);
    NODISCARD const FVulkanPipelineResources* CreateDescriptorSet(const FPipelineResources& desc, FVulkanCommandBatch::FResourceMap& resources);

    NODISCARD FRawRTGeometryID CreateRayTracingGeometry(const FRayTracingGeometryDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawRTSceneID CreateRayTracingScene(const FRayTracingSceneDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD FRawRTShaderTableID CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(FStringView debugName));

    NODISCARD FRawDescriptorSetLayoutID CreateDescriptorSetLayout(const FPipelineDesc::PUniformMap& uniforms ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD FRawSwapchainID CreateSwapchain(const FSwapchainDesc& desc, FRawSwapchainID oldSwapchain, FVulkanFrameGraph& fg ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD bool CreateStagingBuffer(FRawBufferID* pId, FStagingBufferIndex* pIndex, EBufferUsage usage);
    void ReleaseStagingBuffer(FStagingBufferIndex index);


    template <u32 _Uid>
    NODISCARD bool IsResourceAlive(details::TResourceId<_Uid> id) const;
    template <u32 _Uid>
    NODISCARD bool AcquireResource(details::TResourceId<_Uid> id);
    template <u32 _Uid>
    const auto& ResourceData(details::TResourceId<_Uid> id, bool incRef = false) const;
    template <u32 _Uid>
    const auto& ResourceData(const details::TResourceWrappedId<details::TResourceId<_Uid>>& wrappedId, bool incRef = false) const { return ResourceData(wrappedId.Get(), incRef); }
    template <u32 _Uid>
    const auto* ResourceDataIFP(details::TResourceId<_Uid> id, bool incRef = false, bool tolerant = true) const;
    template <u32 _Uid>
    const auto* ResourceDataIFP(const details::TResourceWrappedId<details::TResourceId<_Uid>>& wrappedId, bool incRef = false, bool tolerant = true) const { return ResourceDataIFP(wrappedId.Get(), incRef, tolerant); }
    template <u32 _Uid>
    bool ReleaseResource(details::TResourceId<_Uid> id, u32 refCount = 1);
    void ReleaseResource(FPipelineResources& desc);

    NODISCARD const FBufferDesc& ResourceDescription(FRawBufferID id) const;
    NODISCARD const FImageDesc& ResourceDescription(FRawImageID id) const;

    void RunValidation(u32 maxIteration);
    void ReleaseMemory();

#if USE_PPE_RHIDEBUG
    using FShaderTimemapPipelines = Meta::TArray<FRawCPipelineID, 3>;
    void ShaderTimemapPipelines(FShaderTimemapPipelines* pPpln);

    NODISCARD FRawPipelineLayoutID CreateDebugPipelineLayout(
        FRawPipelineLayoutID baseLayout,
        EShaderDebugMode debugMode,
        EShaderStages debuggableShaders,
        const FDescriptorSetID& descriptorSet );
    NODISCARD FRawDescriptorSetLayoutID CreateDebugDescriptorSetLayout(
        EShaderDebugMode debugMode,
        EShaderStages debuggableShaders,
        FConstChar debugName );

    static u32 DebugShaderStorageSize(EShaderStages stages, EShaderDebugMode mode);
#endif

private:
    struct FResources_ {
        FImagePool ImagePool;
        FBufferPool BufferPool;
        FMemoryObjectPool MemoryObjectPool;
        FSamplerCache SamplerCache;
        FGPipelinePool GPipelinePool;
        FCPipelinePool CPipelinePool;
        FMPipelinePool MPipelinePool;
        FRPipelinePool RPipelinePool;
        FPplnLayoutCache PplnLayoutCache;
        FDSLayoutCache DslayoutCache;
        FPplnResourcesCache PplnResourcesCache;
        FRTGeometryPool RtGeometryPool;
        FRTScenePool RtScenePool;
        FRTShaderTablePool RtShaderTablePool;
        FRenderPassCache RenderPassCache;
        FFramebufferCache FramebufferCache;
        FSwapchainPool SwapchainPool;

        auto& Pool(FRawImageID) { return ImagePool; }
        auto& Pool(FRawBufferID) { return BufferPool; }
        auto& Pool(FRawMemoryID) { return MemoryObjectPool; }
        auto& Pool(FRawSamplerID) { return SamplerCache; }
        auto& Pool(FRawGPipelineID) { return GPipelinePool; }
        auto& Pool(FRawCPipelineID) { return CPipelinePool; }
        auto& Pool(FRawMPipelineID) { return MPipelinePool; }
        auto& Pool(FRawRTPipelineID) { return RPipelinePool; }
        auto& Pool(FRawPipelineLayoutID) { return PplnLayoutCache; }
        auto& Pool(FRawDescriptorSetLayoutID) { return DslayoutCache; }
        auto& Pool(FRawPipelineResourcesID) { return PplnResourcesCache; }
        auto& Pool(FRawRTGeometryID) { return RtGeometryPool; }
        auto& Pool(FRawRTSceneID) { return RtScenePool; }
        auto& Pool(FRawRTShaderTableID) { return RtShaderTablePool; }
        auto& Pool(FRawRenderPassID) { return RenderPassCache; }
        auto& Pool(FRawFramebufferID) { return FramebufferCache; }
        auto& Pool(FRawSwapchainID) { return SwapchainPool; }

        template <u32 _Uid>
        const auto& PoolConst(details::TResourceId<_Uid> id) const {
            return const_cast<FResources_&>(*this).Pool(id);
        }
    };

    template <u32 _Uid>
    using TPooledResource_ = typename Meta::TDecay< decltype(std::declval<FResources_>().Pool(std::declval<details::TResourceId<_Uid>>())) >::value_type;
    template <u32 _Uid>
    auto& ResourcePool_(details::TResourceId<_Uid> id) { return _resources.Pool(id); }
    template <u32 _Uid>
    const auto& ResourcePoolConst_(details::TResourceId<_Uid> id) const { return _resources.PoolConst(id); }

    template <u32 _Uid, typename _Desc>
    NODISCARD bool CreateDevicePipeline_(details::TResourceId<_Uid>* pId, _Desc& desc ARGS_IF_RHIDEBUG(FStringView pipelineType, FConstChar debugName));

    template <u32 _Uid, typename... _Args>
    NODISCARD TPooledResource_<_Uid>* CreatePooledResource_(details::TResourceId<_Uid>* pId, _Args&&... args);
    template <u32 _Uid, typename... _Args>
    NODISCARD TPair<TPooledResource_<_Uid>*, bool> CreateCachedResource_(details::TResourceId<_Uid>* pId, TPooledResource_<_Uid>&& rkey, _Args&&... args);

    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    NODISCARD bool ReleaseResource_(TPool<T, _ChunkSize, _MaxChunks>& pool, T* pdata, FIndex index, u32 refCount);
    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    NODISCARD bool ReleaseResource_(TCache<T, _ChunkSize, _MaxChunks>& cache, T* pdata, FIndex index, u32 refCount);

    NODISCARD bool CreateMemory_(FRawMemoryID* pId, TResourceProxy<FVulkanMemoryObject>** pMemory, const FMemoryDesc& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD bool CreatePipelineLayout_(FRawPipelineLayoutID* pId, const TResourceProxy<FVulkanPipelineLayout>** pLayout, FPipelineDesc::FPipelineLayout&& desc ARGS_IF_RHIDEBUG(FConstChar debugName));
    NODISCARD bool CreatePipelineLayout_(FRawPipelineLayoutID* pId, const TResourceProxy<FVulkanPipelineLayout>** pLayout, const FPipelineDesc::FPipelineLayout& desc, const FDSLayouts& dsLayouts ARGS_IF_RHIDEBUG(FConstChar debugName));

    NODISCARD bool CreateEmptyDescriptorSetLayout_(FRawDescriptorSetLayoutID* pId);
    NODISCARD bool CreateDescriptorSetLayout_(FRawDescriptorSetLayoutID* pId, TResourceProxy<FVulkanDescriptorSetLayout>** pDSLayout, const FPipelineDesc::PUniformMap& uniforms ARGS_IF_RHIDEBUG(FConstChar debugName));

    template <typename _Desc>
    NODISCARD bool CompileShaders_(_Desc& desc);
    NODISCARD bool CompileShaders_(FComputePipelineDesc& desc);
    NODISCARD bool CompileShaderSPIRV_(PVulkanShaderModule* pVkShaderModule, const FShaderDataVariant& spirv);

    NODISCARD bool CheckHostVisibleMemory_();
    void TearDownStagingBuffers_();

    const FVulkanDevice& _device;
    FVulkanMemoryManager _memoryManager;
    FVulkanDescriptorManager _descriptorManager;

    FResources_ _resources;

    std::atomic<u32> _submissionCounter;

    TThreadSafe<FVulkanShaderRefs, EThreadBarrier::CriticalSection> _shaderCache;

    FRawDescriptorSetLayoutID _emptyDSLayout;

    struct {
        FStagingBufferPool Read;
        FStagingBufferPool Write;
        FStagingBufferPool Uniform;

        size_t ReadPageSize{ 0 };
        size_t WritePageSize{ 0 };
        size_t UniformPageSize{ 0 };
        size_t MaxStagingBufferMemory{ UMax };
    }   _staging;

    const FBufferDesc _dummyBufferDesc;
    const FImageDesc _dummyImageDesc;

    struct {
        FFramebufferCache::FGCHandle FrameBuffersGC;
        FPplnResourcesCache::FGCHandle PplnResourcesGC;
        FPplnResourcesCache::FGCHandle RenderPassGC;
    }   _validation;

#if USE_PPE_RHIDEBUG
    using FDebugLayoutCache_ = HASHMAP(RHIResource, u32, FRawDescriptorSetLayoutID);
    struct {
        FDebugLayoutCache_ dsLayoutCaches;
        FCPipelineID PplnFindMaxValue1;
        FCPipelineID PplnFindMaxValue2;
        FCPipelineID PplnTimemapRemap;
    }   _shaderDebug;

    NODISCARD bool CreateFindMaxValuePipeline1_();
    NODISCARD bool CreateFindMaxValuePipeline2_();
    NODISCARD bool CreateTimemapRemapPipeline_();

    void TearDownShaderDebuggerResources_();

#endif

};
//----------------------------------------------------------------------------
template <u32 _Uid>
bool FVulkanResourceManager::IsResourceAlive(details::TResourceId<_Uid> id) const {
    Assert(id);
    auto& pool = ResourcePoolConst_(id);

    if (const auto* const proxyPtr = pool[id.Index])
        return (proxyPtr->IsCreated() && proxyPtr->InstanceID() == id.InstanceID);

    return false;
}
//----------------------------------------------------------------------------
template <u32 _Uid>
bool FVulkanResourceManager::AcquireResource(details::TResourceId<_Uid> id) {
    Assert(id);
    auto& pool = ResourcePoolConst_(id);

    if (const auto* const proxyPtr = pool[id.Index]) {
        if (proxyPtr->IsCreated() && proxyPtr->InstanceID() == id.InstanceID) {
            proxyPtr->AddRef();
            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
template <u32 _Uid>
const auto& FVulkanResourceManager::ResourceData(details::TResourceId<_Uid> id, bool incRef) const {
    const auto *pData = ResourceDataIFP(id, incRef, false);
    Assert(pData);
    return (*pData);
}
//----------------------------------------------------------------------------
template <u32 _Uid>
const auto* FVulkanResourceManager::ResourceDataIFP(details::TResourceId<_Uid> id, bool incRef, bool tolerant) const {
    UNUSED(tolerant);
    auto& pool = ResourcePoolConst_(id);

    using resource_type = typename TPooledResource_<_Uid>::value_type;
    using result_type = const resource_type*;

    if (const TPooledResource_<_Uid>* const pResource = pool[id.Index]) {
        if (pResource->IsCreated() && pResource->InstanceID() == id.InstanceID) {
            if (incRef)
                pResource->AddRef();

            return static_cast<result_type>(&pResource->Data());
        }

        Assert_NoAssume(tolerant || pResource->IsCreated());
        Assert_NoAssume(tolerant || pResource->InstanceID() == id.InstanceID);
    }

    AssertReleaseMessage_NoAssume(L"out-of-bounds", tolerant);
    return static_cast<result_type>(nullptr);
}
//----------------------------------------------------------------------------
template <u32 _Uid>
bool FVulkanResourceManager::ReleaseResource(details::TResourceId<_Uid> id, u32 refCount) {
    Assert(id);
    Assert(refCount > 0);
    auto& pool = ResourcePool_(id);

    using pool_type = Meta::TDecay<decltype(pool)>;
    using proxy_type = typename pool_type::value_type;
    STATIC_ASSERT(std::is_base_of_v<FResourceBaseProxy, proxy_type>);

    if (const proxy_type* const proxyPtr = pool[id.Index]) {
        if (proxyPtr->IsCreated() && proxyPtr->InstanceID() == id.InstanceID)
            return ReleaseResource_(pool, const_cast<proxy_type*>(proxyPtr), id.Index, refCount);
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks>
bool FVulkanResourceManager::ReleaseResource_(TPool<T, _ChunkSize, _MaxChunks>& pool, T* pdata, FIndex index, u32 refCount) {
    Assert(pdata);
    Assert(refCount > 0);

    if (pdata->RemoveRef(refCount)) {
        if (pdata->IsCreated())
            pdata->TearDown(*this);

        Meta::Destroy(pdata);
        pool.Deallocate(index);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
template <typename T, size_t _ChunkSize, size_t _MaxChunks>
bool FVulkanResourceManager::ReleaseResource_(TCache<T, _ChunkSize, _MaxChunks>& cache, T* pdata, FIndex index, u32 refCount) {
    Assert(pdata);
    Assert(refCount > 0);

    return cache.RemoveIf(index, [this, pdata, refCount](T* inCache) {
        UNUSED(pdata);
        Assert_NoAssume(inCache == pdata);

        if (inCache->RemoveRef(refCount)) {
            if (inCache->IsCreated())
                inCache->TearDown(*this);

            return true;
        }

        return false;
    });
}
//----------------------------------------------------------------------------
inline const FBufferDesc& FVulkanResourceManager::ResourceDescription(FRawBufferID id) const {
    Assert(id.Valid());
    const FVulkanBuffer* const pBuffer = ResourceDataIFP(id);
    return (pBuffer ? pBuffer->Read()->Desc : _dummyBufferDesc);
}
//----------------------------------------------------------------------------
inline const FImageDesc& FVulkanResourceManager::ResourceDescription(FRawImageID id) const {
    Assert(id.Valid());
    const FVulkanImage* const pImage = ResourceDataIFP(id);
    return (pImage ? pImage->Read()->Desc : _dummyImageDesc);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
