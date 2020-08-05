#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifndef RHI_VULKAN
#    error "invalid RHI !"
#endif

#if !USE_PPE_RHIDEBUG

#	define PPE_VKDEVICE_SETDEBUGNAME(_VkDevice, _VkElement, _Name) NOOP()
#	define PPE_VKDEVICE_SETDEBUGARGS(_VkDevice, _VkElement, _Fmt, ...) NOOP()

#else

#	include "Color/Color_fwd.h"
#	include "IO/ConstChar.h"
#	include "IO/Format.h"
#	include "IO/TextWriter.h"

#	define PPE_VKDEVICE_SETDEBUGNAME(_RHIDevice, _VkElement, _Name) \
		(_RHIDevice).Debug().SetDebugName(_VkElement, _Name)
#	define PPE_VKDEVICE_SETDEBUGARGS(_RHIDevice, _VkElement, _Fmt, ...) \
		PPE_VKDEVICE_SETDEBUGNAME(_RHIDevice, _VkElement, \
			INLINE_FORMAT(256, _Fmt, __VA_ARGS__).data()/* should be null-terminated */ )

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVulkanDebug : Meta::FNonCopyableNorMovable {
public:
	using string_t = FConstChar;

	explicit FVulkanDebug(VkDevice device);

	void SetObjectName(u64 object, VkDebugReportObjectTypeEXT type, string_t name) const;
	void SetObjectTag(u64 object, VkDebugReportObjectTypeEXT type, u64 name, const FRawMemoryConst& tag) const;

	void BeginRegion(VkCommandBuffer cmdBuffer, string_t name, const FLinearColor& color) const;
	void Marker(VkCommandBuffer cmdBuffer, string_t name, const FLinearColor& color) const;
	void EndRegion(VkCommandBuffer cmdBuffer) const;

	void SetDebugName(VkCommandBuffer cmdBuffer, string_t name) const;
	void SetDebugName(VkQueue queue, string_t name) const;
	void SetDebugName(VkImage image, string_t name) const;
	void SetDebugName(VkSampler sampler, string_t name) const;
	void SetDebugName(VkBuffer buffer, string_t name) const;
	void SetDebugName(VkDeviceMemory memory, string_t name) const;
	void SetDebugName(VkShaderModule shaderModule, string_t name) const;
	void SetDebugName(VkPipeline pipeline, string_t name) const;
	void SetDebugName(VkPipelineLayout pipelineLayout, string_t name) const;
	void SetDebugName(VkRenderPass renderPass, string_t name) const;
	void SetDebugName(VkFramebuffer frameBuffer, string_t name) const;
	void SetDebugName(VkDescriptorSet descriptorSet, string_t name) const;
	void SetDebugName(VkDescriptorSetLayout descriptorSetLayout, string_t name) const;
	void SetDebugName(VkSemaphore semaphore, string_t name) const;
	void SetDebugName(VkFence fence, string_t name) const;
	void SetDebugName(VkEvent _event, string_t name) const;

private:
	using callback_t = void (*)(void);

	VkDevice const _vkDevice;

	callback_t _pfnDebugMarkerSetObjectTag;
	callback_t _pfnDebugMarkerSetObjectName;
	callback_t _pfnCmdDebugMarkerBegin;
	callback_t _pfnCmdDebugMarkerEnd;
	callback_t _pfnCmdDebugMarkerInsert;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
