#include "stdafx.h"

#include "RHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanDebug.h"

#	if USE_PPE_RHIDEBUG

#include "HAL/Vulkan/VulkanRHIIncludes.h"

#include "Color/Color.h"
#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanDebug::FVulkanDebug(VkDevice device)
:	_vkDevice(device)
,	_pfnDebugMarkerSetObjectTag(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT"))
,	_pfnDebugMarkerSetObjectName(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"))
,	_pfnCmdDebugMarkerBegin(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT"))
,	_pfnCmdDebugMarkerEnd(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT"))
,	_pfnCmdDebugMarkerInsert(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT")) {
	Assert_NoAssume(VK_NULL_HANDLE != _vkDevice);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetObjectName(u64 object, VkDebugReportObjectTypeEXT type, string_t name) const {
	Assert((u64)VK_NULL_HANDLE != object);
	Assert(name);

	if (Likely(_pfnDebugMarkerSetObjectName)) {
		VkDebugMarkerObjectNameInfoEXT nameInfo{};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		nameInfo.objectType = type;
		nameInfo.object = object;
		nameInfo.pObjectName = name;
		reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(_pfnDebugMarkerSetObjectName)(_vkDevice, &nameInfo);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetObjectTag(u64 object, VkDebugReportObjectTypeEXT type, u64 name, const FRawMemoryConst& tag) const {
	Assert((u64)VK_NULL_HANDLE != object);

	if (Likely(_pfnDebugMarkerSetObjectTag)) {
		VkDebugMarkerObjectTagInfoEXT tagInfo{};
		tagInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT;
		tagInfo.objectType = type;
		tagInfo.object = object;
		tagInfo.tagName = name;
		tagInfo.tagSize = tag.SizeInBytes();
		tagInfo.pTag = tag.data();
		reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(_pfnDebugMarkerSetObjectTag)(_vkDevice, &tagInfo);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::BeginRegion(VkCommandBuffer cmdBuffer, string_t name, const FLinearColor& color) const {
	Assert(VK_NULL_HANDLE != cmdBuffer);
	Assert(name);

	if (Likely(_pfnCmdDebugMarkerBegin)) {
		VkDebugMarkerMarkerInfoEXT markerInfo{};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		markerInfo.pMarkerName = name;
		markerInfo.color[0] = color.R;
		markerInfo.color[1] = color.G;
		markerInfo.color[2] = color.B;
		markerInfo.color[3] = color.A;
		reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(_pfnCmdDebugMarkerBegin)(cmdBuffer, &markerInfo);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::Marker(VkCommandBuffer cmdBuffer, string_t name, const FLinearColor& color) const {
	Assert(VK_NULL_HANDLE != cmdBuffer);

	if (Likely(_pfnCmdDebugMarkerInsert)) {
		VkDebugMarkerMarkerInfoEXT markerInfo = {};
		markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;
		markerInfo.pMarkerName = name;
		markerInfo.color[0] = color.R;
		markerInfo.color[1] = color.G;
		markerInfo.color[2] = color.B;
		markerInfo.color[3] = color.A;
		reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(_pfnCmdDebugMarkerInsert)(cmdBuffer, &markerInfo);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::EndRegion(VkCommandBuffer cmdBuffer) const {
	Assert(VK_NULL_HANDLE != cmdBuffer);

	if (Likely(_pfnCmdDebugMarkerEnd)) {
		reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(_pfnCmdDebugMarkerEnd)(cmdBuffer);
	}
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkCommandBuffer cmdBuffer, string_t name) const {
	SetObjectName((u64)cmdBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkQueue queue, string_t name) const {
	SetObjectName((u64)queue, VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkImage image, string_t name) const {
	SetObjectName((u64)image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkSampler sampler, string_t name) const {
	SetObjectName((u64)sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkBuffer buffer, string_t name) const {
	SetObjectName((u64)buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkDeviceMemory memory, string_t name) const {
	SetObjectName((u64)memory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkShaderModule shaderModule, string_t name) const {
	SetObjectName((u64)shaderModule, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkPipeline pipeline, string_t name) const {
	SetObjectName((u64)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkPipelineLayout pipelineLayout, string_t name) const {
	SetObjectName((u64)pipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkRenderPass renderPass, string_t name) const {
	SetObjectName((u64)renderPass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkFramebuffer frameBuffer, string_t name) const {
	SetObjectName((u64)frameBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkDescriptorSet descriptorSet, string_t name) const {
	SetObjectName((u64)descriptorSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkDescriptorSetLayout descriptorSetLayout, string_t name) const {
	SetObjectName((u64)descriptorSetLayout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkSemaphore semaphore, string_t name) const {
	SetObjectName((u64)semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkFence fence, string_t name) const {
	SetObjectName((u64)fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name);
}
//----------------------------------------------------------------------------
void FVulkanDebug::SetDebugName(VkEvent _event, string_t name) const {
	SetObjectName((u64)_event, VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, name);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#	endif //!USE_PPE_RHIDEBUG

#endif //!RHI_VULKAN
