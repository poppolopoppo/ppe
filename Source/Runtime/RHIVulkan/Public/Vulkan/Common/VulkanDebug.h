#pragma once

#include "Vulkan/Vulkan_fwd.h"

#include "RHI/FrameDebug.h"

#if !USE_PPE_RHIDEBUG

#	define PPE_VKDEVICE_SETDEBUGNAME(_RHIDevice, _VkElement, _Name) NOOP()
#	define PPE_VKDEVICE_SETDEBUGARGS(_RHIDevice, _VkElement, _Fmt, ...) NOOP()

#else

#	include "Color/Color_fwd.h"
#	include "IO/ConstChar.h"
#	include "IO/Format.h"
#	include "IO/TextWriter.h"
#	include "IO/StaticString.h"

#	define PPE_VKDEVICE_SETDEBUGNAME(_RHIDevice, _vkType, _VkElement, _Name) \
		( (_RHIDevice).Debug().ShouldEmitNames() ? (_RHIDevice).Debug().CONCAT(SetDebugName_, _vkType)(_VkElement, _Name) : void(0) )
#	define PPE_VKDEVICE_SETDEBUGARGS(_RHIDevice, _vkType, _VkElement, _Fmt, ...) \
		PPE_VKDEVICE_SETDEBUGNAME(_RHIDevice, _vkType, _VkElement, \
			INLINE_FORMAT(256, _Fmt, __VA_ARGS__).data()/* should be null-terminated */)

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FVulkanDebugName = FTaskName;
//----------------------------------------------------------------------------
enum class EShaderDebugIndex : u32 { Unknown = ~0u };
//----------------------------------------------------------------------------
class FVulkanDebug : Meta::FNonCopyableNorMovable {
public:
	using string_t = FConstChar;

	explicit FVulkanDebug(const FVulkanDevice& device);

	bool ShouldEmitMarkers() const;
	bool ShouldEmitNames() const;

	void vkSetObjectName(u64 object, VkDebugReportObjectTypeEXT type, string_t name) const;
	void vkSetObjectTag(u64 object, VkDebugReportObjectTypeEXT type, u64 name, const FRawMemoryConst& tag) const;

	void BeginRegion(VkCommandBuffer cmdBuffer, string_t name, const FLinearColor& color) const;
	void Marker(VkCommandBuffer cmdBuffer, string_t name, const FLinearColor& color) const;
	void EndRegion(VkCommandBuffer cmdBuffer) const;

	void SetDebugName_VkCommandBuffer(VkCommandBuffer cmdBuffer, string_t name) const;
	void SetDebugName_VkQueue(VkQueue queue, string_t name) const;
	void SetDebugName_VkImage(VkImage image, string_t name) const;
	void SetDebugName_VkSampler(VkSampler sampler, string_t name) const;
	void SetDebugName_VkBuffer(VkBuffer buffer, string_t name) const;
	void SetDebugName_VkDeviceMemory(VkDeviceMemory memory, string_t name) const;
	void SetDebugName_VkShaderModule(VkShaderModule shaderModule, string_t name) const;
	void SetDebugName_VkPipeline(VkPipeline pipeline, string_t name) const;
	void SetDebugName_VkPipelineLayout(VkPipelineLayout pipelineLayout, string_t name) const;
	void SetDebugName_VkRenderPass(VkRenderPass renderPass, string_t name) const;
	void SetDebugName_VkFramebuffer(VkFramebuffer frameBuffer, string_t name) const;
	void SetDebugName_VkDescriptorSet(VkDescriptorSet descriptorSet, string_t name) const;
	void SetDebugName_VkDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout, string_t name) const;
	void SetDebugName_VkSemaphore(VkSemaphore semaphore, string_t name) const;
	void SetDebugName_VkFence(VkFence fence, string_t name) const;
	void SetDebugName_VkEvent(VkEvent _event, string_t name) const;

private:
	const FVulkanDevice& _device;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
