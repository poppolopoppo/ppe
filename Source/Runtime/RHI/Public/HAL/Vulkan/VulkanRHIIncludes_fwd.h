#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Meta/StronglyTyped.h"

#ifdef RHI_VULKAN

#   ifdef __clang__
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wmicrosoft-enum-forward-reference"
#   endif

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Vulkan
//----------------------------------------------------------------------------
#   ifdef __clang__
#       define PPE_VK_ENUM(_NAME) enum _NAME
#   else
#       define PPE_VK_ENUM(_NAME) enum _NAME : int
#   endif

#define PPE_VK_STRUCT(_NAME) struct _NAME
#define PPE_VK_HANDLE(_NAME) \
    struct CONCAT(_NAME, _T); \
    namespace PPE { namespace RHI { \
        using _NAME = ::CONCAT(_NAME, _T)*; \
    }}

#ifdef ARCH_64BIT
#    define PPE_VK_HANDLE_NON_DISPATCHABLE(_NAME) PPE_VK_HANDLE(_NAME)
#else
    namespace PPE { namespace RHI {
    // keep static type checking with u64 impersonation
        template <typename _Tag>
        struct TVulkanNonDispatchableHandle {
            u64 Handle;

            TVulkanNonDispatchableHandle() = default;

            TVulkanNonDispatchableHandle(const TVulkanNonDispatchableHandle&) = default;
            TVulkanNonDispatchableHandle& operator =(const TVulkanNonDispatchableHandle&) = default;

            TVulkanNonDispatchableHandle(TVulkanNonDispatchableHandle&&) = default;
            TVulkanNonDispatchableHandle& operator =(TVulkanNonDispatchableHandle&&) = default;

            CONSTEXPR TVulkanNonDispatchableHandle(u64 handle) : Handle(handle) {}

            CONSTEXPR operator u64 () const { return Handle; }

            CONSTEXPR u64* operator &() { return &Handle; }
            CONSTEXPR u64& operator *() { return Handle; }

            CONSTEXPR const u64* operator &() const { return &Handle; }
            CONSTEXPR const u64& operator *() const { return Handle; }

            CONSTEXPR bool operator ==(u64 other) const { return (Handle == other); }
            CONSTEXPR bool operator !=(u64 other) const { return (Handle != other); }
        };
    }}
#   define PPE_VK_HANDLE_NON_DISPATCHABLE(_NAME) \
    struct CONCAT(_NAME, _T) {}; \
    namespace PPE { namespace RHI { \
        using _NAME = TVulkanNonDispatchableHandle< CONCAT(_NAME, _T) >; \
    }}
#endif

//----------------------------------------------------------------------------
namespace PPE {
namespace RHI {
using VkFlags = u32;
using VkBool32 = u32;
using VkDeviceSize = u64;
using VkSampleMask = u32;
}}
//----------------------------------------------------------------------------
PPE_VK_ENUM(VkColorSpaceKHR);
PPE_VK_ENUM(VkFormat);
PPE_VK_ENUM(VkPresentModeKHR);
PPE_VK_STRUCT(VkAllocationCallbacks);
//----------------------------------------------------------------------------
PPE_VK_ENUM(VkBlendFactor);
PPE_VK_ENUM(VkBlendOp);
PPE_VK_ENUM(VkColorComponentFlagBits);
PPE_VK_ENUM(VkLogicOp);
PPE_VK_ENUM(VkCompareOp);
PPE_VK_ENUM(VkStencilOp);
PPE_VK_ENUM(VkCullModeFlagBits);
PPE_VK_ENUM(VkFrontFace);
PPE_VK_ENUM(VkPolygonMode);
PPE_VK_ENUM(VkConservativeRasterizationModeEXT);
PPE_VK_ENUM(VkDynamicState);
PPE_VK_ENUM(VkPrimitiveTopology);
PPE_VK_ENUM(VkVertexInputRate);
//----------------------------------------------------------------------------
PPE_VK_ENUM(VkDebugReportObjectTypeEXT);
//----------------------------------------------------------------------------
PPE_VK_HANDLE(VkInstance)
PPE_VK_HANDLE(VkPhysicalDevice)
PPE_VK_HANDLE(VkDevice)
PPE_VK_HANDLE(VkQueue)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkSemaphore)
PPE_VK_HANDLE(VkCommandBuffer)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkFence)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkDeviceMemory)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkBuffer)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkImage)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkEvent)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkQueryPool)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkBufferView)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkImageView)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkShaderModule)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkPipelineCache)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkPipelineLayout)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkRenderPass)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkPipeline)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkDescriptorSetLayout)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkSampler)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkDescriptorPool)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkDescriptorSet)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkFramebuffer)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkCommandPool)
//----------------------------------------------------------------------------
PPE_VK_HANDLE_NON_DISPATCHABLE(VkSwapchainKHR)
PPE_VK_HANDLE_NON_DISPATCHABLE(VkSurfaceKHR)
//----------------------------------------------------------------------------
#undef PPE_VK_HANDLE
#undef PPE_VK_HANDLE_NON_DISPATCHABLE
#undef PPE_VK_ENUM
#undef PPE_VK_STRUCT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

#endif //!RHI_VULKAN
