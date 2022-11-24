// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Descriptors/VulkanPipelineResources.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

#include "RHI/EnumToString.h"

#include "Allocator/SlabAllocator.h"
#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanPipelineResources::FUpdateDescriptors {
    SLAB_ALLOCATOR(RHIDescriptor) Allocator;
    TMemoryView<VkWriteDescriptorSet> DescriptorWrites;
    u32 DescriptorWriteCount{ 0 };

    template <typename T>
    TMemoryView<T> AllocateT(size_t n) NOEXCEPT {
        return TAllocatorTraits<Meta::TDecay<decltype(Allocator)>>::template AllocateT<T>(Allocator, n);
    }

    VkWriteDescriptorSet& NextDescriptorWrite() NOEXCEPT {
        VkWriteDescriptorSet& wds = DescriptorWrites[DescriptorWriteCount++];
        wds = {};
        wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        return wds;
    }

    bool AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FBuffer& value);
    bool AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FTexelBuffer& value);
    bool AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FImage& value);
    bool AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FTexture& value);
    bool AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, const FPipelineResources::FSampler& value);
    bool AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, const FPipelineResources::FRayTracingScene& value);
};
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::FUpdateDescriptors::AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FBuffer& value) {
    Unused(id);
    const auto infos = AllocateT<VkDescriptorBufferInfo>(value.Elements.Count);

    forrange(i, 0, value.Elements.Count) {
        auto& elt = value.Elements[i];
        const FVulkanBuffer* const pBuffer = manager.ResourceDataIFP(elt.BufferId, false, true);

        if (Unlikely(not pBuffer)) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        VkDescriptorBufferInfo& info = infos[i];
        info.buffer = pBuffer->Handle();
        info.offset = static_cast<VkDeviceSize>(elt.Offset);
        info.range = static_cast<VkDeviceSize>(elt.Size);

        CheckBufferUsage(*pBuffer, value.State);
    }

    const bool isUniform{ Meta::EnumAnd(value.State, EResourceState::_StateMask) == EResourceState::UniformRead };
    const bool isDynamic{ value.State & EResourceState::_BufferDynamicOffset };

    VkWriteDescriptorSet& wds = NextDescriptorWrite();
    wds.descriptorType = (isUniform
        ? (isDynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        : (isDynamic ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER));
    wds.descriptorCount = value.Elements.Count;
    wds.dstBinding = value.Index.VKBinding();
    wds.dstSet = data.DescriptorSet.First;
    wds.pBufferInfo = infos.data();

    RHI_TRACE(L"UpdateDescriptors_Buffer", id, Fmt::Struct(wds));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::FUpdateDescriptors::AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FTexelBuffer& value) {
    Unused(id);
    const auto infos = AllocateT<VkBufferView>(value.Elements.Count);

    forrange(i, 0, value.Elements.Count) {
        auto& elt = value.Elements[i];
        const FVulkanBuffer* const pBuffer = manager.ResourceDataIFP(elt.BufferId, false, true);

        if (Unlikely(not pBuffer)) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        VkBufferView& info = infos[i];
        info = pBuffer->MakeView(manager.Device(), elt.Desc);

        if (Unlikely(VK_NULL_HANDLE == info)) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        CheckBufferUsage(*pBuffer, value.State);
    }

    const bool isUniform{ Meta::EnumAnd(value.State, EResourceState::_StateMask) == EResourceState::UniformRead };

    VkWriteDescriptorSet& wds = NextDescriptorWrite();
    wds.descriptorType = (isUniform
        ? VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
        : VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
    wds.descriptorCount = value.Elements.Count;
    wds.dstBinding = value.Index.VKBinding();
    wds.dstSet = data.DescriptorSet.First;
    wds.pTexelBufferView = infos.data();

    RHI_TRACE(L"UpdateDescriptors_TexelBuffer", id, Fmt::Struct(wds));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::FUpdateDescriptors::AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FImage& value) {
    Unused(id);
    const auto infos = AllocateT<VkDescriptorImageInfo>(value.Elements.Count);

    forrange(i, 0, value.Elements.Count) {
        auto& elt = value.Elements[i];
        const FVulkanImage* const pImage = manager.ResourceDataIFP(elt.ImageId, false, true);

        if (Unlikely(not pImage)) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        VkDescriptorImageInfo& info = infos[i];
        info.imageLayout = EResourceState_ToImageLayout(value.State, pImage->Read()->AspectMask);
        info.imageView = pImage->MakeView(manager.Device(), elt.Desc);
        info.sampler = VK_NULL_HANDLE;

        if (Unlikely(VK_NULL_HANDLE == info.imageView)) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        AssertRelease(elt.Desc.has_value());
        CheckImageType(id, i, *pImage, *elt.Desc, value.ImageType);
        CheckImageUsage(*pImage, value.State);
    }

    const bool isInputAttachment{ Meta::EnumAnd(value.State, EResourceState::_StateMask) == EResourceState::InputAttachment };

    VkWriteDescriptorSet& wds = NextDescriptorWrite();
    wds.descriptorType = (isInputAttachment
        ? VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
        : VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
    wds.descriptorCount = value.Elements.Count;
    wds.dstBinding = value.Index.VKBinding();
    wds.dstSet = data.DescriptorSet.First;
    wds.pImageInfo = infos.data();

    RHI_TRACE(L"UpdateDescriptors_Image", id, Fmt::Struct(wds));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::FUpdateDescriptors::AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, FPipelineResources::FTexture& value) {
    Unused(id);
    const auto infos = AllocateT<VkDescriptorImageInfo>(value.Elements.Count);

    forrange(i, 0, value.Elements.Count) {
        auto& elt = value.Elements[i];
        const FVulkanImage* const pImage = manager.ResourceDataIFP(elt.ImageId, false, true);
        const FVulkanSampler* const pSampler = manager.ResourceDataIFP(elt.SamplerId, false, true);

        if (Unlikely(not (pImage and pSampler))) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        VkDescriptorImageInfo& info = infos[i];
        info.imageLayout = EResourceState_ToImageLayout(value.State, pImage->Read()->AspectMask);
        info.imageView = pImage->MakeView(manager.Device(), elt.Desc);
        info.sampler = pSampler->Handle();

        if (Unlikely(VK_NULL_HANDLE == info.imageView)) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        AssertRelease(elt.Desc.has_value());
        CheckTextureType(id, i, *pImage, *elt.Desc, value.SamplerType);
        CheckImageUsage(*pImage, value.State);
    }

    VkWriteDescriptorSet& wds = NextDescriptorWrite();
    wds.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    wds.descriptorCount = value.Elements.Count;
    wds.dstBinding = value.Index.VKBinding();
    wds.dstSet = data.DescriptorSet.First;
    wds.pImageInfo = infos.data();

    RHI_TRACE(L"UpdateDescriptors_Texture", id, Fmt::Struct(wds));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::FUpdateDescriptors::AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, const FPipelineResources::FSampler& value) {
    Unused(id);
    const auto infos = AllocateT<VkDescriptorImageInfo>(value.Elements.Count);

    forrange(i, 0, value.Elements.Count) {
        auto& elt = value.Elements[i];
        const FVulkanSampler* const pSampler = manager.ResourceDataIFP(elt.SamplerId, false, true);

        if (Unlikely(not pSampler)) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        VkDescriptorImageInfo& info = infos[i];
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.imageView = VK_NULL_HANDLE;
        info.sampler = pSampler->Handle();
    }

    VkWriteDescriptorSet& wds = NextDescriptorWrite();
    wds.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    wds.descriptorCount = value.Elements.Count;
    wds.dstBinding = value.Index.VKBinding();
    wds.dstSet = data.DescriptorSet.First;
    wds.pImageInfo = infos.data();

    RHI_TRACE(L"UpdateDescriptors_Sampler", id, Fmt::Struct(wds));
    return true;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::FUpdateDescriptors::AddResource(FInternalResources& data, FVulkanResourceManager& manager, const FUniformID& id, const FPipelineResources::FRayTracingScene& value) {
    Unused(id);
    const auto tlas = AllocateT<VkAccelerationStructureNV>(value.Elements.Count);

    forrange(i, 0, value.Elements.Count) {
        auto& elt = value.Elements[i];
        const FVulkanRTScene* const pRTScene = manager.ResourceDataIFP(elt.SceneId, false, true);

        if (Unlikely(not pRTScene)) {
            ONLY_IF_RHIDEBUG(ValidateEmptyUniform_(data, id, i));
            return false;
        }

        tlas[i] = pRTScene->Handle();
    }

    auto* const pTopAS = AllocateT<VkWriteDescriptorSetAccelerationStructureNV>(1).data();
    *pTopAS = {};
    pTopAS->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
    pTopAS->accelerationStructureCount = value.Elements.Count;
    pTopAS->pAccelerationStructures = tlas.data();

    VkWriteDescriptorSet& wds = NextDescriptorWrite();
    wds.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    wds.pNext = pTopAS;
    wds.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
    wds.descriptorCount = value.Elements.Count;
    wds.dstBinding = value.Index.VKBinding();
    wds.dstSet = data.DescriptorSet.First;

    RHI_TRACE(L"UpdateDescriptors_RayTracingScene", id, Fmt::Struct(wds));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanPipelineResources::FInternalResources::FInternalResources(
    FDynamicData&& rdynamicData,
    FRawDescriptorSetLayoutID layoutId,
    bool allowEmptyResources ) NOEXCEPT
:   LayoutId(layoutId)
,   DynamicData(std::move(rdynamicData))
,   AllowEmptyResources(allowEmptyResources) {
    Assert_NoAssume(LayoutId.Valid());
    HashValue = hash_tuple(LayoutId,
        FPipelineResources::ComputeDynamicDataHash(DynamicData) );
}
//----------------------------------------------------------------------------
FVulkanPipelineResources::FVulkanPipelineResources(const FPipelineResources& desc)
:   _resources(FInternalResources{
        FPipelineResources::CloneDynamicData(desc),
        desc.Layout(),
        desc.AllowEmptyResources()
    })
#if USE_PPE_RHIDEBUG
,   _debugName(desc.DebugName())
#endif
{

}
//----------------------------------------------------------------------------
FVulkanPipelineResources::FVulkanPipelineResources(FPipelineResources&& rdesc) NOEXCEPT
:   _resources(FInternalResources{
        FPipelineResources::StealDynamicData(rdesc),
        rdesc.Layout(),
        rdesc.AllowEmptyResources()
    })
#if USE_PPE_RHIDEBUG
,   _debugName(rdesc.DebugName())
#endif
{

}
//----------------------------------------------------------------------------
FVulkanPipelineResources::FVulkanPipelineResources(FVulkanPipelineResources&& rvalue) NOEXCEPT
:   _resources(std::move(*rvalue.Write()))
#if USE_PPE_RHIDEBUG
,   _debugName(rvalue._debugName)
#endif
{

}
//----------------------------------------------------------------------------
FVulkanPipelineResources::~FVulkanPipelineResources() {
    Assert_NoAssume(VK_NULL_HANDLE == _resources.LockExclusive()->DescriptorSet.First);
}
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::Construct(FVulkanResourceManager& manager) {
    const auto exclusiveRes = _resources.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE == exclusiveRes->DescriptorSet.First);

    const FVulkanDescriptorSetLayout* const pDsLayout = manager.ResourceDataIFP(exclusiveRes->LayoutId, true);
    LOG_CHECK(RHI, !!pDsLayout );

    const FVulkanDevice& device = manager.Device();

#ifdef VK_EXT_robustness2
    // Without the nullDescriptor feature enabled, when updating a VkDescriptorSet, all the resources backing it must be non-null,
    // even if the descriptor is statically not used by the shader. This feature allows descriptors to be backed by null resources or views.
    // Loads from a null descriptor return zero values and stores and atomics to a null descriptor are discarded.
    // https://github.com/KhronosGroup/Vulkan-Guide/blob/master/chapters/robustness.md
    if (device.Enabled().Robustness2 and device.Capabilities().Robustness2Features.nullDescriptor) {
        // exclusiveRes->AllowEmptyResources can be true
    }
    else {
        exclusiveRes->AllowEmptyResources = false;
    }
#endif

    LOG_CHECK(RHI, pDsLayout->AllocateDescriptorSet(
        &exclusiveRes->DescriptorSet, manager ));

    SLABHEAP_POOLED(RHIDescriptor) heap;
    heap.SetSlabSize(16_KiB);

    FUpdateDescriptors update{
        heap,
        heap.AllocateT<VkWriteDescriptorSet>(pDsLayout->Read()->MaxIndex + 1),
        0 };

    exclusiveRes->DynamicData.EachUniform([&](const FUniformID& id, auto& data) {
        Assert_NoAssume(id.Valid());
        update.AddResource(*exclusiveRes, manager, id, data);
    });

    device.vkUpdateDescriptorSets(
        device.vkDevice(),
        update.DescriptorWriteCount,
        update.DescriptorWrites.data(),
        0, nullptr );

    heap.DiscardAll();
    return true;
}
//----------------------------------------------------------------------------
void FVulkanPipelineResources::TearDown(FVulkanResourceManager& manager) {
    const auto exclusiveRes = _resources.LockExclusive();

    const FVulkanDescriptorSetLayout* const pDsLayout = manager.ResourceDataIFP(exclusiveRes->LayoutId, false, true);

    if (pDsLayout and exclusiveRes->DescriptorSet.First)
        pDsLayout->DeallocateDescriptorSet(manager, exclusiveRes->DescriptorSet);

    // release reference only if descriptor set was created
    if (exclusiveRes->LayoutId and exclusiveRes->DescriptorSet.First)
        manager.ReleaseResource(exclusiveRes->LayoutId);

    exclusiveRes->DynamicData = FDynamicData{};
    exclusiveRes->DescriptorSet = { VK_NULL_HANDLE, UMax };
    exclusiveRes->LayoutId = Default;
    exclusiveRes->HashValue = Default;
}
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::AllResourcesAlive(const FVulkanResourceManager& manager) const {
    const auto sharedRes = _resources.LockShared();

    const auto isNotAlive = [&manager](const auto& id) {
        return (id && not manager.IsResourceAlive(id));
    };

    return (not sharedRes->DynamicData.UniformByPred(Meta::Overloaded(
        [&isNotAlive](const FUniformID&, const FPipelineResources::FBuffer& buf) {
            return buf.Elements.MakeView().Any([&isNotAlive](const FPipelineResources::FBuffer::FElement& elt) {
                return isNotAlive(elt.BufferId);
            }).valid();
        },
        [&isNotAlive](const FUniformID&, const FPipelineResources::FTexelBuffer& texel) {
            return texel.Elements.MakeView().Any([&isNotAlive](const FPipelineResources::FTexelBuffer::FElement& elt) {
                return isNotAlive(elt.BufferId);
            }).valid();
        },
        [&isNotAlive](const FUniformID&, const FPipelineResources::FImage& img) {
            return img.Elements.MakeView().Any([&isNotAlive](const FPipelineResources::FImage::FElement& elt) {
                return isNotAlive(elt.ImageId);
            }).valid();
        },
        [&isNotAlive](const FUniformID&, const FPipelineResources::FTexture& tex) {
            return tex.Elements.MakeView().Any([&isNotAlive](const FPipelineResources::FTexture::FElement& elt) {
                return isNotAlive(elt.ImageId) and isNotAlive(elt.SamplerId);
            }).valid();
        },
        [&isNotAlive](const FUniformID&, const FPipelineResources::FSampler& smp) {
            return smp.Elements.MakeView().Any([&isNotAlive](const FPipelineResources::FSampler::FElement& elt) {
                return isNotAlive(elt.SamplerId);
            }).valid();
        },
        [&isNotAlive](const FUniformID&, const FPipelineResources::FRayTracingScene& scene) {
            return scene.Elements.MakeView().Any([&isNotAlive](const FPipelineResources::FRayTracingScene::FElement& elt) {
                return isNotAlive(elt.SceneId);
            }).valid();
        }) ));
}
//----------------------------------------------------------------------------
bool FVulkanPipelineResources::operator ==(const FVulkanPipelineResources& other) const NOEXCEPT {
    const auto lhs = _resources.LockShared();
    const auto rhs = other._resources.LockShared();

    return (lhs->HashValue == rhs->HashValue and
            lhs->LayoutId == rhs->LayoutId and
            FPipelineResources::CompareDynamicData(lhs->DynamicData, rhs->DynamicData) );
}
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
void FVulkanPipelineResources::ValidateEmptyUniform_(const FInternalResources& data, const FUniformID& id, u32 idx) {
    Unused(data);
    Unused(id);
    Unused(idx);
    if (not data.AllowEmptyResources)
        RHI_LOG(Error, L"uniform '{0}' [{1}] contains invalid resource(s)!", id.MakeView(), idx);
}
#endif
//----------------------------------------------------------------------------
void FVulkanPipelineResources::CheckBufferUsage(const FVulkanBuffer& buffer, EResourceState state) {
#if USE_PPE_RHIDEBUG
    const EBufferUsage usage = buffer.Read()->Desc.Usage;

    switch (Meta::EnumAnd(state, EResourceState::_AccessMask)) {
    case EResourceState::_Access_ShaderStorage: AssertRelease(usage & EBufferUsage::Storage); break;
    case EResourceState::_Access_Uniform: AssertRelease(usage & EBufferUsage::Uniform); break;
    default: AssertReleaseFailed(L"unknown resource state");
    }
#else
    Unused(buffer);
    Unused(state);
#endif
}
//----------------------------------------------------------------------------
void FVulkanPipelineResources::CheckTexelBufferUsage(const FVulkanBuffer& buffer, EResourceState state) {
#if USE_PPE_RHIDEBUG
    const EBufferUsage usage = buffer.Read()->Desc.Usage;

    switch (Meta::EnumAnd(state, EResourceState::_AccessMask)) {
    case EResourceState::_Access_ShaderStorage: AssertRelease((usage & EBufferUsage::StorageTexel) || (usage & EBufferUsage::StorageTexelAtomic)); break;
    case EResourceState::_Access_Uniform: AssertRelease(usage & EBufferUsage::UniformTexel); break;
    default: AssertReleaseFailed(L"unknown resource state");
    }
#else
    Unused(buffer);
    Unused(state);
#endif
}
//----------------------------------------------------------------------------
void FVulkanPipelineResources::CheckImageUsage(const FVulkanImage& image, EResourceState state) {
#if USE_PPE_RHIDEBUG
    const EImageUsage usage = image.Read()->Desc.Usage;

    switch (Meta::EnumAnd(state, EResourceState::_AccessMask)) {
    case EResourceState::_Access_ShaderStorage: AssertRelease((usage & EImageUsage::Storage) || (usage & EImageUsage::StorageAtomic)); break;
    case EResourceState::_Access_ShaderSample: AssertRelease(usage & EImageUsage::Sampled); break;
    default: AssertReleaseFailed(L"unknown resource state");
    }
#else
    Unused(image);
    Unused(state);
#endif
}
//----------------------------------------------------------------------------
void FVulkanPipelineResources::CheckImageType(const FUniformID& id, u32 index, const FVulkanImage& img, const FImageViewDesc& desc, EImageSampler shaderType) {
#if USE_PPE_RHIDEBUG
    Assert(id.Valid());
    const EPixelFormat fmt = img.Read()->Desc.Format;

    if (fmt != Default && fmt != desc.Format) {
        RHI_LOG(Error,
            L"Incompatible image formats in uniform {0}[{1}]:\n"
            L"  in shader: {2}\n"
            L"  in image : {3}, name: {4}\n",
            id.MakeView(), index,
            fmt, desc.Format,
            img.DebugName());
    }

    CheckTextureType(id, index, img, desc, shaderType - EImageSampler::_FormatMask);
#else
    Unused(id);
    Unused(index);
    Unused(img);
    Unused(desc);
    Unused(shaderType);
#endif
}
//----------------------------------------------------------------------------
void FVulkanPipelineResources::CheckTextureType(const FUniformID& id, u32 index, const FVulkanImage& img, const FImageViewDesc& desc, EImageSampler shaderType) {
#if USE_PPE_RHIDEBUG
    Assert(id.Valid());
    Assert(not (shaderType ^ EImageSampler::_FormatMask));
    Unused(img);
    Unused(index);

    EImageSampler imageType = static_cast<EImageSampler>(0);

    switch (desc.View) {
    case EImageView::_1D: imageType = EImageSampler::_1D; break;
    case EImageView::_2D: imageType = EImageSampler::_2D; break;
    case EImageView::_3D: imageType = EImageSampler::_3D; break;
    case EImageView::_1DArray: imageType = EImageSampler::_1DArray; break;
    case EImageView::_2DArray: imageType = EImageSampler::_2DArray; break;
    case EImageView::_Cube: imageType = EImageSampler::_Cube; break;
    case EImageView::_CubeArray: imageType = EImageSampler::_CubeArray; break;
    case EImageView::Unknown: AssertNotReached();
    }

    const FPixelFormatInfo info = EPixelFormat_Infos(desc.Format);

    if (desc.AspectMask == EImageAspect::Stencil) {
        Assert((info.ValueType & EPixelValueType::Stencil) ||
            (info.ValueType & EPixelValueType::DepthStencil));
        imageType |= EImageSampler::_Int;
    }
    else
    if (desc.AspectMask == EImageAspect::Depth) {
        Assert((info.ValueType & EPixelValueType::Depth) ||
            (info.ValueType & EPixelValueType::DepthStencil));
        imageType |= EImageSampler::_Float;
    }
    else
    if (info.ValueType & EPixelValueType::Int) {
        imageType |= EImageSampler::_Int;
    }
    else
    if (info.ValueType & EPixelValueType::UInt) {
        imageType |= EImageSampler::_UInt;
    }
    else
    if (info.ValueType ^ EPixelValueType::AnyFloat) {
        imageType |= EImageSampler::_Float;
    }
    else {
        AssertReleaseFailed(L"unknown pixel value type");
    }

    if (imageType != shaderType) {
        RHI_LOG(Error,
            L"Incompatible image formats in uniform {0}[{1}]:\n"
            L"  in shader: {2}\n"
            L"  in image : {3}, name: {4}\n",
            id.MakeView(), index,
            shaderType, imageType,
            img.DebugName());
    }

#else
    Unused(id);
    Unused(index);
    Unused(img);
    Unused(desc);
    Unused(shaderType);
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
