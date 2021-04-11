#include "stdafx.h"

#include "RHI_fwd.h"

#include "Vulkan/Common/VulkanDebug.h"

#	if USE_PPE_RHIDEBUG

#include "Vulkan/VulkanIncludes.h"

#include "Vulkan/VulkanDevice.h"

#include "Color/Color.h"
#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
CONSTEXPR static u64 VulkanObject_(u64 nonDispatchableHandle) {
	return nonDispatchableHandle;
}
//----------------------------------------------------------------------------
template <typename T>
static u64 VulkanObject_(const T* handle) {
	return static_cast<u64>(reinterpret_cast<uintptr_t>(handle));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDebug::FVulkanDebug(const FVulkanDevice& device)
:	_device(device)
{}
//----------------------------------------------------------------------------
bool FVulkanDebug::ShouldEmitMarkers() const {
	return (!!_device.vkCmdDebugMarkerBeginEXT & !!_device.vkCmdDebugMarkerInsertEXT & !!_device.vkCmdDebugMarkerEndEXT);
}
//----------------------------------------------------------------------------
bool FVulkanDebug::ShouldEmitNames() const {
	return (!!_device.vkDebugMarkerSetObjectNameEXT & !!_device.vkDebugMarkerSetObjectTagEXT);
}
//----------------------------------------------------------------------------
void FVulkanDebug::vkSetObjectName(u64 object, VkDebugReportObjectTypeEXT type, string_t name) const {
	Assert(VulkanObject_(VK_NULL_HANDLE) != object);
	Assert(name);

	if (_device.vkDebugMarkerSetObjectNameEXT) {
		VkDebugMarkerObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = type;
		nameInfo.object = object;
		nameInfo.pObjectName = name;

		_device.vkDebugMarkerSetObjectNameEXT(_device.vkDevice(), &nameInfo);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::vkSetObjectTag(u64 object, VkDebugReportObjectTypeEXT type, u64 name, const FRawMemoryConst& tag) const {
	Assert(VulkanObject_(VK_NULL_HANDLE) != object);

	if (_device.vkDebugMarkerSetObjectTagEXT) {
		VkDebugMarkerObjectTagInfoEXT tagInfo{};
		tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
		tagInfo.objectType = type;
		tagInfo.object = object;
		tagInfo.tagName = name;
		tagInfo.tagSize = tag.SizeInBytes();
		tagInfo.pTag = tag.data();

		_device.vkDebugMarkerSetObjectTagEXT(_device.vkDevice(), &tagInfo);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::BeginRegion(VkCommandBuffer cmdBuffer, string_t name, const FLinearColor& color) const {
	Assert(VK_NULL_HANDLE != cmdBuffer);
	Assert(name);

	if (_device.vkCmdDebugMarkerBeginEXT) {
		VkDebugMarkerMarkerInfoEXT markerInfo{};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		markerInfo.pMarkerName = name;
		markerInfo.color[0] = color.R;
		markerInfo.color[1] = color.G;
		markerInfo.color[2] = color.B;
		markerInfo.color[3] = color.A;

		_device.vkCmdDebugMarkerBeginEXT(cmdBuffer, &markerInfo);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::Marker(VkCommandBuffer cmdBuffer, string_t name, const FLinearColor& color) const {
	Assert(VK_NULL_HANDLE != cmdBuffer);

	if (_device.vkCmdDebugMarkerInsertEXT) {
		VkDebugMarkerMarkerInfoEXT markerInfo = {};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		markerInfo.pMarkerName = name;
		markerInfo.color[0] = color.R;
		markerInfo.color[1] = color.G;
		markerInfo.color[2] = color.B;
		markerInfo.color[3] = color.A;

		_device.vkCmdDebugMarkerInsertEXT(cmdBuffer, &markerInfo);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::EndRegion(VkCommandBuffer cmdBuffer) const {
	Assert(VK_NULL_HANDLE != cmdBuffer);

	if (_device.vkCmdDebugMarkerEndEXT)
		_device.vkCmdDebugMarkerEndEXT(cmdBuffer);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkCommandBuffer(VkCommandBuffer cmdBuffer, string_t name) const {
	vkSetObjectName(VulkanObject_(cmdBuffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkQueue(VkQueue queue, string_t name) const {
	vkSetObjectName(VulkanObject_(queue), VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkImage(VkImage image, string_t name) const {
	vkSetObjectName(VulkanObject_(image), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkSampler(VkSampler sampler, string_t name) const {
	vkSetObjectName(VulkanObject_(sampler), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkBuffer(VkBuffer buffer, string_t name) const {
	vkSetObjectName(VulkanObject_(buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkDeviceMemory(VkDeviceMemory memory, string_t name) const {
	vkSetObjectName(VulkanObject_(memory), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkShaderModule(VkShaderModule shaderModule, string_t name) const {
	vkSetObjectName(VulkanObject_(shaderModule), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkPipeline(VkPipeline pipeline, string_t name) const {
	vkSetObjectName(VulkanObject_(pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkPipelineLayout(VkPipelineLayout pipelineLayout, string_t name) const {
	vkSetObjectName(VulkanObject_(pipelineLayout), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkRenderPass(VkRenderPass renderPass, string_t name) const {
	vkSetObjectName(VulkanObject_(renderPass), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkFramebuffer(VkFramebuffer frameBuffer, string_t name) const {
	vkSetObjectName(VulkanObject_(frameBuffer), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkDescriptorSet(VkDescriptorSet descriptorSet, string_t name) const {
	vkSetObjectName(VulkanObject_(descriptorSet), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout, string_t name) const {
	vkSetObjectName(VulkanObject_(descriptorSetLayout), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkSemaphore(VkSemaphore semaphore, string_t name) const {
	vkSetObjectName(VulkanObject_(semaphore), VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkFence(VkFence fence, string_t name) const {
	vkSetObjectName(VulkanObject_(fence), VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName_VkEvent(VkEvent _event, string_t name) const {
	vkSetObjectName(VulkanObject_(_event), VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, name);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#	endif //!USE_PPE_RHIDEBUG
