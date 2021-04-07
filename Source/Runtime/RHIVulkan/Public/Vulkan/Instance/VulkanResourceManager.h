#pragma once

#include "Vulkan/Vulkan_fwd.h"

#include "Vulkan/VulkanCommon.h"
#include "Vulkan/Descriptors/VulkanDescriptorManager.h"
#include "Vulkan/Memory/VulkanMemoryManager.h"

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
    using FIndex = FRawImageID::index_t;

    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    using TPool = TTypedMemoryPool<T, _ChunkSize, _MaxChunks, true, ALLOCATOR(RHIResource)>;
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

    using FStagingBufferPool    = TAtomicPool<FBufferID, u32>;

    using FPipelineCompilers    = HASHSET(RHIResource, PPipelineCompiler);
    using FVulkanShaderRefs     = VECTOR(RHIResource, PVulkanShaderModule);
    using FDSLayouts            = TFixedSizeStack<TPair<
        FRawDescriptorSetLayoutID,
        TResourceProxy<FVulkanDescriptorSetLayout>
    >,  MaxDescriptorSets >;

#if USE_PPE_RHIDEBUG
    using FDebugLayoutCache = HASHMAP(RHIResource, u32, FRawDescriptorSetLayoutID);
#endif

    explicit FVulkanResourceManager(const FVulkanDevice& device);
    ~FVulkanResourceManager();

    const FVulkanDevice& Device() const { return _device; }
    FVulkanMemoryManager& MemoryManager() { return _memoryManager; }
    FVulkanDescriptorManager& DescriptorManager() { return _descriptorManager; }

    u32 SubmitIndex() const { return _submissionCounter.load(std::memory_order_relaxed); }

    u32 HostReadBufferSize() const { return _staging.ReadPageSize; }
    u32 HostWriteBufferSize() const { return _staging.WritePageSize; }
    u32 UniformBufferSize() const { return _staging.UniformPageSize; }


    bool Create();
    void TearDown();


    FRawMPipelineID CreatePipeline(FMeshPipelineDesc& desc ARGS_IF_RHIDEBUG(const FStringView& debugName));
    FRawGPipelineID CreatePipeline(FGraphicsPipelineDesc& desc ARGS_IF_RHIDEBUG(const FStringView& debugName));
    FRawCPipelineID CreatePipeline(FComputePipelineDesc& desc ARGS_IF_RHIDEBUG(const FStringView& debugName));
    FRawRTPipelineID CreatePipeline(FRayTracingPipelineDesc& desc ARGS_IF_RHIDEBUG(const FStringView& debugName));

    FRawImageID CreateImage(const FImageDesc& desc, const FMemoryDesc& mem, EVulkanQueueFamilyMask queues, EResourceState defaultState ARGS_IF_RHIDEBUG(const FStringView& name));
    FRawBufferID CreateBuffer(const FBufferDesc& desc, const FMemoryDesc& mem, EVulkanQueueFamilyMask queues ARGS_IF_RHIDEBUG(const FStringView& name));
    FRawSamplerID CreateSampler(const FSamplerDesc& desc  ARGS_IF_RHIDEBUG(const FStringView& name));

    FRawRenderPassID CreateRenderPass(const TMemoryView<const FVulkanLogicalRenderPass>& passes ARGS_IF_RHIDEBUG(const FStringView& name));
    FRawFramebufferID CreateFramebuffer(const TMemoryView<const TPair<FRawImageID, FImageViewDesc>>& attachments, FRawRenderPassID pass, const uint2& dim, u32 layers ARGS_IF_RHIDEBUG(const FStringView& name));

    bool CacheDescriptorSet(FPipelineResources& desc);
    const FVulkanPipelineResources* CreateDescriptorSet(const FPipelineResources& desc, FVulkanCommandBatch::FResourceMap& resources ARGS_IF_RHIDEBUG(const FStringView& name));

    FRawRTGeometryID CreateRayTracingGeometry(const FRayTracingGeometryDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(const FStringView& name));
    FRawRTSceneID CreateRayTracingScene(const FRayTracingSceneDesc& desc, const FMemoryDesc& mem ARGS_IF_RHIDEBUG(const FStringView& name));
    FRawRTShaderTableID CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG(const FStringView& name));

    FRawDescriptorSetLayoutID CreateDescriptorSetLayout(const FPipelineDesc::FSharedUniformMap& uniforms);

    FRawSwapchainID CreateSwapchain(const FVulkanSwapchainDesc& desc, FRawSwapchainID oldSwapchain, FVulkanFrameGraph& fg ARGS_IF_RHIDEBUG(const FStringView& name));


    bool CreateStagingBuffer(FRawBufferID* pId, FStagingBufferIndex* pIndex, EBufferUsage usage);
    void ReleaseStagingBuffer(FStagingBufferIndex index);


    template <u32 _Uid>
    bool IsResourceAlive(details::TResourceId<_Uid> id) const;
    template <u32 _Uid>
    bool AcquireResource(details::TResourceId<_Uid> id);
    template <u32 _Uid>
    const auto* ResourceData(details::TResourceId<_Uid> id, bool incRef = false, bool tolerant = false) const;
    template <u32 _Uid>
    bool ReleaseResource(details::TResourceId<_Uid> id, u32 refCount = 1);
    void ReleaseResource(FPipelineResources& desc);

    const FBufferDesc& ResourceDescription(FRawBufferID id) const;
    const FImageDesc& ResourceDescription(FRawImageID id) const;


    void RunValidation(u32 maxIteration);


#if USE_PPE_RHIDEBUG
    void ShaderTimemapPipelines(Meta::TArray<FRawCPipelineID, 3>* pPpln);

    FRawPipelineLayoutID CreateDebugPipelineLayout(
        FRawPipelineLayoutID baseLayout,
        EShaderDebugMode debugMode,
        EShaderStages debuggableShaders,
        const FDescriptorSetID& descriptorSet );
    FRawDescriptorSetLayoutID CreateDebugDescriptorSetLayout(
        EShaderDebugMode debugMode,
        EShaderStages debuggableShaders );

    static u32 DebugShaderStorageSize(EShaderStages stages, EShaderDebugMode mode);
#endif

private:
    auto& ResourcePool_(FRawImageID) { return _imagePool; }
    auto& ResourcePool_(FRawBufferID) { return _bufferPool; }
    auto& ResourcePool_(FRawMemoryID) { return _memoryObjectPool; }
    auto& ResourcePool_(FRawSamplerID) { return _samplerCache; }
    auto& ResourcePool_(FRawGPipelineID) { return _gpipelinePool; }
    auto& ResourcePool_(FRawCPipelineID) { return _cpipelinePool; }
    auto& ResourcePool_(FRawMPipelineID) { return _mpipelinePool; }
    auto& ResourcePool_(FRawRTPipelineID) { return _rpipelinePool; }
    auto& ResourcePool_(FRawPipelineLayoutID) { return _pplnLayoutCache; }
    auto& ResourcePool_(FRawDescriptorSetLayoutID) { return _dslayoutCache; }
    auto& ResourcePool_(FRawPipelineResourcesID) { return _pplnResourcesCache; }
    auto& ResourcePool_(FRawRTGeometryID) { return _rtgeometryPool; }
    auto& ResourcePool_(FRawRTSceneID) { return _rtscenePool; }
    auto& ResourcePool_(FRawRTShaderTableID) { return _rtshaderTablePool; }
    auto& ResourcePool_(FRawRenderPassID) { return _renderPassCache; }
    auto& ResourcePool_(FRawFramebufferID) { return _framebufferCache; }
    auto& ResourcePool_(FRawSwapchainID) { return _swapchainPool; }

    template <u32 _Uid>
    const auto& ResourcePoolConst_(details::TResourceId<_Uid> id) const {
        return const_cast<FVulkanResourceManager&>(*this).ResourcePool_(id);
    }

    template <u32 _Uid, typename _OnInit, typename _OnCreate>
    details::TResourceId<_Uid> CreateCachedResource_(FStringView errorMsg, _OnInit&& onInit, _OnCreate&& onCreate);
    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    bool ReleaseResource_(TPool<T, _ChunkSize, _MaxChunks>& pool, T* pdata, u32 index, u32 refCount);
    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    bool ReleaseResource_(TCache<T, _ChunkSize, _MaxChunks>& cache, T* pdata, u32 index, u32 refCount);
    template <typename T, size_t _ChunkSize, size_t _MaxChunks>
    void TearDownCache_(TCache<T, _ChunkSize, _MaxChunks>& cache);


    bool CreateMemory_(FRawMemoryID* pId, TResourceProxy<FVulkanMemoryObject>** pMemRef, const FMemoryDesc& desc ARGS_IF_RHIDEBUG(const FStringView& name));
    bool CreatePipelineLayout_(FRawPipelineLayoutID* pId, const TResourceProxy<FVulkanPipelineLayout>** pPplnLayoutRef, FPipelineDesc::FPipelineLayout&& desc);
    bool CreatePipelineLayout_(FRawPipelineLayoutID* pId, const TResourceProxy<FVulkanPipelineLayout>** pPplnLayoutRef, const FPipelineDesc::FPipelineLayout& desc, const FDSLayouts& dslayouts);

    bool CreateEmptyDescriptorSetLayout_(FRawDescriptorSetLayoutID* pId);
    bool CreateDescriptorSetLayout_(FRawDescriptorSetLayoutID* pId, TResourceProxy<FVulkanDescriptorSetLayout>** pDSLayoutRef, const FPipelineDesc::FSharedUniformMap& uniforms);

    template <typename _Desc>
    bool CompileShaders_(_Desc& desc);
    bool CompileShaders_(FComputePipelineDesc& desc);
    bool CompileShaderSPIRV_(PVulkanShaderModule* pVkShaderModule, const FShaderSource& source);

    bool CheckHostVisibleMemory_();
    void TearDownStagingBuffers_();


    const FVulkanDevice& _device;
    FVulkanMemoryManager _memoryManager;
    FVulkanDescriptorManager _descriptorManager;

    FImagePool _imagePool;
    FBufferPool _bufferPool;
    FMemoryObjectPool _memoryObjectPool;
    FSamplerCache _samplerCache;
    FGPipelinePool _gpipelinePool;
    FCPipelinePool _cpipelinePool;
    FMPipelinePool _mpipelinePool;
    FRPipelinePool _rpipelinePool;
    FPplnLayoutCache _pplnLayoutCache;
    FDSLayoutCache _dslayoutCache;
    FPplnResourcesCache _pplnResourcesCache;
    FRTGeometryPool _rtgeometryPool;
    FRTScenePool _rtscenePool;
    FRTShaderTablePool _rtshaderTablePool;
    FRenderPassCache _renderPassCache;
    FFramebufferCache _framebufferCache;
    FSwapchainPool _swapchainPool;

    std::atomic<u32> _submissionCounter;

    FCriticalSection _shaderCacheCS;
    FVulkanShaderRefs _shaderCache;

    FReadWriteLock _compilersRW;
    FPipelineCompilers _compilers;

    FRawDescriptorSetLayoutID _emptyDSLayout;

    struct {
        FStagingBufferPool Read;
        FStagingBufferPool Write;
        FStagingBufferPool Uniform;

        u32 ReadPageSize{ 0 };
        u32 WritePageSize{ 0 };
        u32 UniformPageSize{ 0 };
    }   _staging;

    struct {
        std::atomic_uint CreatedFramebuffers{ 0 };
        std::atomic_uint LastCheckedFramebuffer{ 0 };
        std::atomic_uint CreatedPplnResources{ 0 };
        std::atomic_uint LastCheckedPplnResource{ 0 };
    }   _validation;

    const FBufferDesc _dummyBufferDesc;
    const FImageDesc _dummyImageDesc;

#if USE_PPE_RHIDEBUG
    using FDebugLayoutCache_ = HASHMAP(RHIResource, u32, FRawDescriptorSetLayoutID);
    struct {
        FDebugLayoutCache_ dsLayoutCaches;
        FCPipelineID PplnFindMaxValue1;
        FCPipelineID PplnFindMaxValue2;
        FCPipelineID PplnRemap;
    }   _shaderDebug;

    bool CreateFindMaxValuePipeline1_();
    bool CreateFindMaxValuePipeline2_();
    bool CreateTimemapRemapPipeline_();

    void TearDownShaderDebuggerResources_();

#endif

};
//----------------------------------------------------------------------------
template <u32 _Uid>
bool FVulkanResourceManager::IsResourceAlive(details::TResourceId<_Uid> id) const {
    Assert(id);
    auto& pool = ResourcePoolConst_(id);

    if (const auto* const proxyPtr = pool[id.Index]) {
        return (proxyPtr->IsCreated() && proxyPtr->InstanceID() == id.InstanceID);
    }

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
const auto* FVulkanResourceManager::ResourceData(details::TResourceId<_Uid> id, bool incRef, bool tolerant) const {
    Assert(id);
    auto& pool = ResourcePoolConst_(id);

    using pool_type = Meta::TDecay<decltype(pool)>;
    using proxy_type = typename pool_type::value_type;
    STATIC_ASSERT(std::is_base_of_v<FResourceBase, proxy_type>);
    using resource_type = typename proxy_type::value_type;
    using result_type = const resource_type*;

    const proxy_type* const proxyPtr = pool[id.Index];
    if (proxyPtr) {
        if (proxyPtr->IsCreated() && proxyPtr->InstanceId() == id.InstanceID) {
            if (incRef)
                proxyPtr->AddRef();

            return static_cast<result_type>(&proxyPtr->Data());
        }

        Assert_NoAssume(tolerant || proxyPtr->IsCreated());
        Assert_NoAssume(tolerant || proxyPtr->InstanceID() == id.InstanceID);
    }

    AssertReleaseMessage_NoAssume(L"out-of-bounds", tolerant);
    return static_cast<result_type>(nullptr);
}
//----------------------------------------------------------------------------
template <u32 _Uid>
bool FVulkanResourceManager::ReleaseResource(details::TResourceId<_Uid> id, u32 refCount) {
    Assert(id);
    Assert(refCount);
    auto& pool = ResourcePoolConst_(id);

    using pool_type = Meta::TDecay<decltype(pool)>;
    using proxy_type = typename pool_type::value_type;
    STATIC_ASSERT(std::is_base_of_v<FResourceBase, proxy_type>);

    if (const proxy_type* const proxyPtr = pool[id.Index]) {
        if (proxyPtr->IsCreated() && proxyPtr->InstanceID() == id.InstanceID)
            return ReleaseResource_(pool, const_cast<proxy_type*>(proxyPtr), id.Index, refCount);
    }

    return false;
}
template <typename T, size_t _ChunkSize, size_t _MaxChunks>
bool FVulkanResourceManager::ReleaseResource_(TPool<T, _ChunkSize, _MaxChunks>& pool, T* pdata, u32 index, u32 refCount) {
    Assert(pdata);

    if (pdata->RemoveRef(refCount)) {
        if (pdata->IsCreated())
            pdata->TearDown(*this);

        pool.Deallocate(index);
        return true;
    }

    return false;
}
template <typename T, size_t _ChunkSize, size_t _MaxChunks>
bool FVulkanResourceManager::ReleaseResource_(TCache<T, _ChunkSize, _MaxChunks>& cache, T* pdata, u32 index, u32 refCount) {
    Assert(pdata);

    return cache.RemoveIf(index, [this, pdata, refCount](T* inCache) {
        UNUSED(pdata);
        Assert_NoAssume(inCache == pdata);

        if (inCache->RemoveRef(refCount)) {
            if (inCache->IsCreated())
                inCache->TearDown(*this);
        }

        return false;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
