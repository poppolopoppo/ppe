// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Vulkan/Image/VulkanImage.h"

#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"

#include "Diagnostic/Logger.h"
#include "Maths/ScalarVectorHelpers.h"

#if USE_PPE_RHIDEBUG
#   include "RHI/EnumToString.h"
#   include "Vulkan/Common/VulkanEnumToString.h"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
NODISCARD static VkImageAspectFlagBits ChooseAspect_(EPixelFormat fmt) {
    VkImageAspectFlagBits result = Zero;

    if (EPixelFormat_IsColor(fmt)) {
        result |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    else {
        if (EPixelFormat_HasDepth(fmt))
            result |= VK_IMAGE_ASPECT_DEPTH_BIT;
        if (EPixelFormat_HasStencil(fmt))
            result |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    return result;
}
//----------------------------------------------------------------------------
NODISCARD static VkImageLayout ChooseDefaultLayout_(EImageUsage usage, VkImageLayout defaultLayout) {
    VkImageLayout result = VK_IMAGE_LAYOUT_GENERAL;

    if (defaultLayout != VK_IMAGE_LAYOUT_MAX_ENUM and defaultLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
        result = defaultLayout;
    }
    else
    // render target layouts has high priority to avoid unnecessary decompressions
    if (usage & EImageUsage::ColorAttachment) {
        result = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    else
    if (usage & EImageUsage::DepthStencilAttachment) {
        result = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    else
    if (usage & EImageUsage::Sampled) {
        result = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    else
    if (usage & EImageUsage::Storage) {
        result = VK_IMAGE_LAYOUT_GENERAL;
    }

    return result;
}
//----------------------------------------------------------------------------
NODISCARD static VkAccessFlagBits AllImageAccessMasks_(VkImageUsageFlags usage) {
    VkAccessFlagBits result = Zero;

    for (VkImageUsageFlags t = 1; t <= usage; t <<= 1) {
        if ((usage & t) != t)
            continue;

        switch (static_cast<VkImageUsageFlagBits>(t)) {
        case VK_IMAGE_USAGE_TRANSFER_SRC_BIT: result |= VK_ACCESS_TRANSFER_READ_BIT; break;
        case VK_IMAGE_USAGE_TRANSFER_DST_BIT: break;
        case VK_IMAGE_USAGE_SAMPLED_BIT: result |= VK_ACCESS_SHADER_READ_BIT; break;
        case VK_IMAGE_USAGE_STORAGE_BIT: result |= VK_ACCESS_SHADER_READ_BIT; break;
        case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: result |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;  break;
        case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT: result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT; break;
        case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT: break;
        case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT: result |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT; break;

#ifdef VK_EXT_fragment_density_map
        case VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT: break;
#endif
#ifdef VK_NV_shading_rate_image
        case VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV: result |= VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV; break;
#endif

        default: AssertNotReached();
        }
    }

    return result;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_ASSERT
FVulkanImage::~FVulkanImage() {
    const auto exclusiveData = _data.LockExclusive();
    Assert_NoAssume(VK_NULL_HANDLE == exclusiveData->vkImage);
    Assert_NoAssume(not exclusiveData->MemoryId.Valid());
    Assert_NoAssume(_viewMap.LockExclusive()->empty());
}
#endif
//----------------------------------------------------------------------------
FVulkanImage::FVulkanImage(FVulkanImage&& rvalue) NOEXCEPT
:   _data(std::move(*rvalue._data.LockExclusive()))
,   _viewMap(std::move(*rvalue._viewMap.LockExclusive()))
{}
//----------------------------------------------------------------------------
bool FVulkanImage::Construct(
    FVulkanResourceManager& resources,
    const FImageDesc& desc,
    FRawMemoryID memoryId,
    FVulkanMemoryObject& memoryObject,
    EVulkanQueueFamilyMask queueFamilyMask,
    EResourceState defaultState
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    const auto exclusiveData = _data.LockExclusive();
    Assert(VK_NULL_HANDLE == exclusiveData->vkImage);
    Assert(not exclusiveData->MemoryId.Valid());
    Assert(not desc.IsExternal);
    Assert(desc.Format != Default);
    Assert(desc.Usage != Default);

    const FVulkanDevice& device = resources.Device();

    exclusiveData->Desc = desc;
    exclusiveData->Desc.Validate();
    exclusiveData->MemoryId = FMemoryID{ memoryId };

    AssertRelease( IsSupported(device, exclusiveData->Desc, static_cast<EMemoryType>(memoryObject.MemoryType())) );

    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = VkCast(exclusiveData->Desc.Flags);
    info.imageType = VkCast(exclusiveData->Desc.Type);
    info.format = VkCast(exclusiveData->Desc.Format);
    info.extent.width = exclusiveData->Desc.Dimensions.x;
    info.extent.height = exclusiveData->Desc.Dimensions.y;
    info.extent.depth = exclusiveData->Desc.Dimensions.z;
    info.mipLevels = *exclusiveData->Desc.MaxLevel;
    info.arrayLayers = *exclusiveData->Desc.ArrayLayers;
    info.samples = VkCast(exclusiveData->Desc.Samples);
    info.usage = VkCast(exclusiveData->Desc.Usage);

    const bool optTiling = (not (memoryObject.MemoryType() ^ EVulkanMemoryType::HostVisible));
    if (optTiling) {
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    else {
        info.tiling = VK_IMAGE_TILING_LINEAR;
        info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    }

    // setup sharing mode

    TStaticArray<u32, MaxQueueFamilies> queueFamilyIndices = {};

    if (queueFamilyMask != Default) {
        info.sharingMode = VK_SHARING_MODE_CONCURRENT;
        info.pQueueFamilyIndices = queueFamilyIndices.data();

        for (u32 i = 0, mask = (1u << i);
             mask <= static_cast<u32>(queueFamilyMask) and info.queueFamilyIndexCount < queueFamilyIndices.size();
             ++i, mask = (1u << i) ) {

            if (Meta::EnumHas(queueFamilyMask, mask))
                queueFamilyIndices[info.queueFamilyIndexCount++] = i;
        }
    }

    // reset exclusive mode

    if (info.queueFamilyIndexCount < 2) {
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.pQueueFamilyIndices = nullptr;
        info.queueFamilyIndexCount = 0;
    }

    VK_CHECK(device.vkCreateImage(
        device.vkDevice(),
        &info,
        device.vkAllocator(),
        &exclusiveData->vkImage ));

    LOG_CHECK(RHI, memoryObject.AllocateImage(resources.MemoryManager(), exclusiveData->vkImage));

    exclusiveData->AspectMask = ChooseAspect_(exclusiveData->Desc.Format);
    exclusiveData->DefaultLayout = ChooseDefaultLayout_(
        exclusiveData->Desc.Usage,
        EResourceState_ToImageLayout(defaultState, exclusiveData->AspectMask) );
    exclusiveData->QueueFamilyMask = queueFamilyMask;
    exclusiveData->ReadAccessMask = AllImageAccessMasks_(info.usage);

#if USE_PPE_RHIDEBUG
    if (debugName) {
        _debugName = debugName;
        device.SetObjectName(exclusiveData->vkImage, _debugName, VK_OBJECT_TYPE_IMAGE);
    }
#endif

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanImage::Construct(
    const FVulkanDevice& device,
    const FVulkanExternalImageDesc& desc,
    FOnReleaseExternalImage&& onRelease
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    const auto exclusiveData = _data.LockExclusive();
    Assert(VK_NULL_HANDLE == exclusiveData->vkImage);
    Assert(VK_NULL_HANDLE != desc.Image);

    exclusiveData->vkImage = desc.Image;
    exclusiveData->Desc.Type = RHICast(desc.Type);
    exclusiveData->Desc.Flags = RHICast(desc.Flags);
    exclusiveData->Desc.Dimensions = desc.Dimensions;
    exclusiveData->Desc.Format = RHICast(desc.Format);
    exclusiveData->Desc.Usage = RHICast(desc.Usage);
    exclusiveData->Desc.ArrayLayers = FImageLayer{ desc.ArrayLayers };
    exclusiveData->Desc.MaxLevel = FMipmapLevel{ desc.MaxLevels };
    exclusiveData->Desc.Samples = RHICast(desc.Samples);
    exclusiveData->Desc.IsExternal = true;

    exclusiveData->Desc.Validate();

    LOG_CHECK(RHI, IsSupported(device, exclusiveData->Desc, EMemoryType::Default));

#if USE_PPE_RHIDEBUG
    if (debugName) {
        _debugName = debugName;
        device.SetObjectName(exclusiveData->vkImage, _debugName, VK_OBJECT_TYPE_IMAGE);
    }
#endif

    LOG_CHECK(RHI, VK_QUEUE_FAMILY_IGNORED == desc.QueueFamily); // not supported
    LOG_CHECK(RHI, desc.ConcurrentQueueFamilyIndices.empty() || desc.ConcurrentQueueFamilyIndices.size() >= 2);

    exclusiveData->QueueFamilyMask = Default;
    for (u32 index : desc.ConcurrentQueueFamilyIndices)
        exclusiveData->QueueFamilyMask |= bit_cast<EVulkanQueueFamily>(index);

    exclusiveData->AspectMask = ChooseAspect_(exclusiveData->Desc.Format);
    exclusiveData->DefaultLayout = ChooseDefaultLayout_(exclusiveData->Desc.Usage, desc.DefaultLayout);
    exclusiveData->OnRelease = std::move(onRelease);

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanImage::Construct(
    const FVulkanDevice& device,
    const FImageDesc& desc,
    FExternalImage externalImage,
    FOnReleaseExternalImage&& onRelease,
    TMemoryView<const u32> queueFamilyIndices,
    EResourceState defaultState
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    const auto exclusiveData = _data.LockExclusive();
    Assert(VK_NULL_HANDLE == exclusiveData->vkImage);
    Assert(desc.IsExternal);
    Assert(externalImage);
    AssertMessage_NoAssume(L"not supported", desc.Queues == EQueueUsage::Unknown);
    AssertMessage_NoAssume(L"not supported", queueFamilyIndices.empty() or queueFamilyIndices.size() >= 2);

    exclusiveData->vkImage = FVulkanExternalObject(externalImage.Value).Cast<VkImage>();
    exclusiveData->Desc = desc;

    AssertRelease( IsSupported(device, exclusiveData->Desc, EMemoryType::Default) );

    exclusiveData->QueueFamilyMask = Zero;
    for (u32 index : queueFamilyIndices)
        exclusiveData->QueueFamilyMask |= bit_cast<EVulkanQueueFamily>(index);

    exclusiveData->AspectMask = ChooseAspect_(exclusiveData->Desc.Format);
    exclusiveData->DefaultLayout = ChooseDefaultLayout_(exclusiveData->Desc.Usage,
        EResourceState_ToImageLayout(defaultState, exclusiveData->AspectMask) );
    exclusiveData->OnRelease = std::move(onRelease);

#if USE_PPE_RHIDEBUG
    if (debugName) {
        _debugName = debugName;
        device.SetObjectName(exclusiveData->vkImage, _debugName, VK_OBJECT_TYPE_IMAGE);
    }
#endif

    return true;
}
//----------------------------------------------------------------------------
void FVulkanImage::TearDown(FVulkanResourceManager& resources) {
    const auto exclusiveData = _data.LockExclusive();
    Assert(VK_NULL_HANDLE != exclusiveData->vkImage);

    const FVulkanDevice& device = resources.Device();

#if USE_PPE_RHIDEBUG
    if (_debugName && exclusiveData->vkImage)
        device.SetObjectName(exclusiveData->vkImage, nullptr, VK_OBJECT_TYPE_IMAGE);
#endif

    {
        const auto exclusiveViewMap = _viewMap.LockExclusive();

        for (const auto& it : *exclusiveViewMap) {
            device.vkDestroyImageView(device.vkDevice(), it.second, device.vkAllocator() );
        }

        exclusiveViewMap->clear();
    }

    if (exclusiveData->Desc.IsExternal) {
        if (exclusiveData->OnRelease)
            exclusiveData->OnRelease(FVulkanExternalObject(exclusiveData->vkImage).ExternalImage());
    }
    else
        device.vkDestroyImage(device.vkDevice(), exclusiveData->vkImage, device.vkAllocator() );

    if (exclusiveData->MemoryId)
        resources.ReleaseResource(exclusiveData->MemoryId.Release());

    exclusiveData->vkImage = VK_NULL_HANDLE;
    exclusiveData->MemoryId = Default;
    exclusiveData->Desc = Default;
    exclusiveData->AspectMask = Zero;
    exclusiveData->DefaultLayout = Zero;
    exclusiveData->QueueFamilyMask = Default;
    exclusiveData->OnRelease = NoFunction;

    ONLY_IF_RHIDEBUG(_debugName.Clear());
}
//----------------------------------------------------------------------------
VkImageView FVulkanImage::MakeView(const FVulkanDevice& device, const FImageViewDescMemoized& desc) const {
    const auto sharedData = _data.LockShared();

    // find already create image view
    {
        const auto sharedViewMap = _viewMap.LockShared();

        const auto it = sharedViewMap->find(desc);
        if (sharedViewMap->end() != it)
            return it->second;
    }

    // create a new image view

    const auto exclusiveViewMap = _viewMap.LockExclusive();

    auto[it, inserted] = exclusiveViewMap->insert({ desc, VK_NULL_HANDLE });
    if (inserted) {
        LOG_CHECK(RHI, IsSupported(device, desc) );
        LOG_CHECK(RHI, CreateView_(&it->second, *sharedData, device, desc ARGS_IF_RHIDEBUG(_debugName)) );
    }

    Assert_NoAssume(VK_NULL_HANDLE != it->second);
    return it->second;
}
//----------------------------------------------------------------------------
VkImageView FVulkanImage::MakeView(const FVulkanDevice& device, FImageViewDesc& desc) const {
    desc.Validate(_data.LockShared()->Desc);

    return MakeView(device, Memoize(desc));
}
//----------------------------------------------------------------------------
VkImageView FVulkanImage::MakeView(const FVulkanDevice& device, Meta::TOptional<FImageViewDesc>& desc) const {
    if (not desc.has_value())
        desc = FImageViewDesc{ _data.LockShared()->Desc };
    else
        desc->Validate(_data.LockShared()->Desc);

    return MakeView(device, Memoize(*desc));
}
//----------------------------------------------------------------------------
bool FVulkanImage::FInternalData::IsReadOnly() const {
    return (not (Desc.Usage ^ (
        EImageUsage::TransferDst | EImageUsage::ColorAttachment | EImageUsage::Storage |
        EImageUsage::DepthStencilAttachment | EImageUsage::TransientAttachment |
        EImageUsage::ColorAttachmentBlend | EImageUsage::StorageAtomic )) );
}
//----------------------------------------------------------------------------
bool FVulkanImage::IsSupported(const FVulkanDevice& device, const FImageDesc& desc, EMemoryType memoryType) {
    const bool optTiling = (not (memoryType ^ (EMemoryType::HostRead | EMemoryType::HostWrite)));
    const VkFormat vkFormat = VkCast(desc.Format);

    // check available creation flags
    {
        const VkImageCreateFlagBits required = VkCast(desc.Flags);
        const VkImageCreateFlagBits available = device.Flags().ImageCreateFlags;

        if (not (available & required))
            return false;
    }

    // check format features
    {
        VkFormatProperties props{};
        device.vkGetPhysicalDeviceFormatProperties(device.vkPhysicalDevice(), vkFormat, &props);

        VkFormatFeatureFlags required = 0;
        const VkFormatFeatureFlags available = (optTiling
            ? props.optimalTilingFeatures
            : props.linearTilingFeatures );

        for (u32 t = 1; t <= static_cast<u32>(desc.Usage); t <<= 1) {
            if (not Meta::EnumHas(desc.Usage, t))
                continue;

            switch (static_cast<EImageUsage>(t)) {
            case EImageUsage::TransferSrc: required |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT; break;
            case EImageUsage::TransferDst: required |= VK_FORMAT_FEATURE_TRANSFER_DST_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT; break;
            case EImageUsage::Sampled: required |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;break;
            case EImageUsage::Storage: required |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT; break;
            case EImageUsage::SampledMinMax: required |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT_EXT; break;
            case EImageUsage::StorageAtomic: required |= VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT; break;
            case EImageUsage::ColorAttachment: required |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT; break;
            case EImageUsage::ColorAttachmentBlend: required |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT; break;
            case EImageUsage::DepthStencilAttachment: required |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT; break;

            case EImageUsage::TransientAttachment: AssertNotImplemented(); // TODO
            case EImageUsage::InputAttachment: break;

            case EImageUsage::ShadingRate:
                if (not device.Enabled().ShadingRateImageNV)
                    return false;
                break;

            case EImageUsage::_Last:
            case EImageUsage::All:
            case EImageUsage::Transfer:
            case EImageUsage::Unknown: AssertNotReached();
            }
        }

        if ((available & required) != required)
            return false;
    }

    // check image properties
    {
        VkImageFormatProperties props{};
        VK_CHECK(device.vkGetPhysicalDeviceImageFormatProperties(
            device.vkPhysicalDevice(),
            vkFormat,
            VkCast(desc.Type),
            optTiling
                ? VK_IMAGE_TILING_OPTIMAL
                : VK_IMAGE_TILING_LINEAR,
            VkCast(desc.Usage),
            VkCast(desc.Flags),
            &props ));

        if (desc.Dimensions.x > props.maxExtent.width or
            desc.Dimensions.y > props.maxExtent.height or
            desc.Dimensions.z > props.maxExtent.depth )
            return false;

        if (*desc.MaxLevel > props.maxArrayLayers)
            return false;

        if ((props.sampleCounts & *desc.Samples) != *desc.Samples)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanImage::IsSupported(const FVulkanDevice& device, const FImageViewDesc& desc) const {
    const auto sharedData = _data.LockShared();

    if (desc.View == EImageView_CubeArray) {
        if (not device.Features().imageCubeArray)
            return false;

        if (sharedData->Desc.Type != EImageDim_2D or (sharedData->Desc.Type == EImageDim_3D and sharedData->Desc.Flags & EImageFlags::Array2DCompatible))
            return false;

        if (not (sharedData->Desc.Flags & EImageFlags::CubeCompatible))
            return false;

        if (desc.LayerCount % 6 != 0)
            return false;
    }

    if (desc.View == EImageView_Cube) {
        if (not (sharedData->Desc.Flags & EImageFlags::CubeCompatible))
            return false;

        if (desc.LayerCount != 6)
            return false;
    }

    if (sharedData->Desc.Type == EImageDim_3D and desc.View != EImageView_3D) {
        if (not (sharedData->Desc.Flags & EImageFlags::Array2DCompatible))
            return false;
    }

    if (desc.Format != Default and desc.Format != sharedData->Desc.Format) {
        if (not (sharedData->Desc.Flags & EImageFlags::MutableFormat))
            return false;

        const FPixelFormatInfo required = EPixelFormat_Infos(sharedData->Desc.Format);
        const FPixelFormatInfo original = EPixelFormat_Infos(desc.Format);

        const bool requiredUseBlock = AnyLess(required.BlockDim, uint2::One);
        const bool originalUseBlock = AnyLess(original.BlockDim, uint2::One);

        // compressed to uncompressed
        if (sharedData->Desc.Flags & EImageFlags::BlockTexelViewCompatible and originalUseBlock and not requiredUseBlock) {
            if (required.BitsPerBlock0 != original.BitsPerBlock0)
                return false;
        }
        else {
            if (requiredUseBlock != originalUseBlock)
                return false;

            if (required.BlockDim != original.BlockDim)
                return false;

            if (desc.AspectMask == EImageAspect::Stencil) {
                if (required.BitsPerBlock1 != original.BitsPerBlock1)
                    return false;
            }
            else {
                if (required.BitsPerBlock0 != original.BitsPerBlock0)
                    return false;
            }
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVulkanImage::CreateView_(
    VkImageView* pImageView,
    const FInternalData& data,
    const FVulkanDevice& device,
    const FImageViewDescMemoized& desc
    ARGS_IF_RHIDEBUG(const FVulkanDebugName& debugName) ) {
    Assert(pImageView);
    CONSTEXPR const VkComponentSwizzle vkComponents[] = {
        VK_COMPONENT_SWIZZLE_IDENTITY, // unknown
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A,
        VK_COMPONENT_SWIZZLE_ZERO,
        VK_COMPONENT_SWIZZLE_ONE
    };

    const uint4 swizzle = Min(
        uint4(checked_cast<u32>(lengthof(vkComponents) - 1)),
        Unswizzle(desc->Swizzle) );

    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.viewType = VkCast(desc->View);
    info.flags = 0;
    info.image = data.vkImage;
    info.format = VkCast(desc->Format);
    info.components = {
        vkComponents[swizzle.x],
        vkComponents[swizzle.y],
        vkComponents[swizzle.z],
        vkComponents[swizzle.w],
    };

    info.subresourceRange.aspectMask = VkCast(desc->AspectMask);
    info.subresourceRange.baseMipLevel = *desc->BaseLevel;
    info.subresourceRange.levelCount = desc->LevelCount;
    info.subresourceRange.baseArrayLayer = *desc->BaseLayer;
    info.subresourceRange.layerCount = desc->LayerCount;

    VK_CHECK(device.vkCreateImageView(
        device.vkDevice(),
        &info,
        device.vkAllocator(),
        pImageView ));

#if USE_PPE_RHIDEBUG
    if (debugName)
        device.SetObjectName(FVulkanExternalObject{ *pImageView }, INLINE_FORMAT(256, "{0}-{1}-{2}(layer={3}:{4},level={5}:{6})", debugName, desc.Value().Format, desc.Value().AspectMask, desc.Value().BaseLayer, desc.Value().LayerCount, desc.Value().BaseLevel, desc.Value().LevelCount), VK_OBJECT_TYPE_IMAGE_VIEW);
#endif

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
