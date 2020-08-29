#include "stdafx.h"

#include "RHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Vulkan/VulkanError.h"

#include "Diagnostic/Logger.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/Format.h"

#include "HAL/Vulkan/VulkanRHIIncludes.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_RHI_API, RHI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanException::FVulkanException(const char* what)
:   FVulkanException(what, VK_SUCCESS)
{}
//----------------------------------------------------------------------------
FVulkanException::FVulkanException(const char* what, long errorCode)
:   FRHIException(what)
,   _errorCode(errorCode) {
#if !USE_PPE_FINAL_RELEASE
    LOG(RHI, Error, L"Vulkan: {0} failed with {1}", MakeCStringView(what), FVulkanError(_errorCode));
    FLUSH_LOG();
#endif
}
//----------------------------------------------------------------------------
FVulkanException::~FVulkanException() = default;
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FVulkanException::Description(FWTextWriter& oss) const {
    return oss
        << MakeCStringView(What()) << L": "
        << FVulkanError(_errorCode);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FVulkanError& error) {
    switch (VkResult(error.Code)) {
    case VK_SUCCESS: return oss << "VK_SUCCESS";
    case VK_NOT_READY: return oss << "VK_NOT_READY";
    case VK_TIMEOUT: return oss << "VK_TIMEOUT";
    case VK_EVENT_SET: return oss << "VK_EVENT_SET";
    case VK_EVENT_RESET: return oss << "VK_EVENT_RESET";
    case VK_INCOMPLETE: return oss << "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return oss << "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return oss << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return oss << "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return oss << "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return oss << "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return oss << "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return oss << "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return oss << "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return oss << "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return oss << "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return oss << "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL: return oss << "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN: return oss << "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY: return oss << "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE: return oss << "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION: return oss << "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return oss << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_SURFACE_LOST_KHR: return oss << "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return oss << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return oss << "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return oss << "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return oss << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return oss << "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV: return oss << "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_INCOMPATIBLE_VERSION_KHR: return oss << "VK_ERROR_INCOMPATIBLE_VERSION_KHR";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return oss << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT: return oss << "VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return oss << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR: return oss << "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR: return oss << "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR: return oss << "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR: return oss << "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT: return oss << "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT";
    case VK_RESULT_MAX_ENUM: return oss << "VK_RESULT_MAX_ENUM";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FVulkanError& error) {
    switch (VkResult(error.Code)) {
    case VK_SUCCESS: return oss << L"VK_SUCCESS";
    case VK_NOT_READY: return oss << L"VK_NOT_READY";
    case VK_TIMEOUT: return oss << L"VK_TIMEOUT";
    case VK_EVENT_SET: return oss << L"VK_EVENT_SET";
    case VK_EVENT_RESET: return oss << L"VK_EVENT_RESET";
    case VK_INCOMPLETE: return oss << L"VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return oss << L"VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return oss << L"VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return oss << L"VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return oss << L"VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return oss << L"VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return oss << L"VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return oss << L"VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return oss << L"VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return oss << L"VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return oss << L"VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return oss << L"VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL: return oss << L"VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN: return oss << L"VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY: return oss << L"VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE: return oss << L"VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION: return oss << L"VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return oss << L"VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_SURFACE_LOST_KHR: return oss << L"VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return oss << L"VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return oss << L"VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return oss << L"VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return oss << L"VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return oss << L"VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV: return oss << L"VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_INCOMPATIBLE_VERSION_KHR: return oss << L"VK_ERROR_INCOMPATIBLE_VERSION_KHR";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return oss << L"VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT: return oss << L"VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return oss << L"VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR: return oss << L"VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR: return oss << L"VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR: return oss << L"VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR: return oss << L"VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT: return oss << L"VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT";
    case VK_RESULT_MAX_ENUM: return oss << L"VK_RESULT_MAX_ENUM";
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
